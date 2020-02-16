/*
 * Copyright (c) 2018-2019, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <gst/gst.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "deepstream_app.h"

#define MAX_DISPLAY_LEN 64
static guint batch_num = 0;

GST_DEBUG_CATEGORY_EXTERN (NVDS_APP);

GQuark _dsmeta_quark;

#define CEIL(a,b) ((a + b - 1) / b)
#define SOURCE_RESET_INTERVAL_IN_MS 60000

/**
 * @brief  Add the (nvmsgconv->nvmsgbroker) sink-bin to the
 *         overall DS pipeline (if any configured) and link the same to
 *         common_elements.tee (This tee connects
 *         the common analytics path to Tiler/display-sink and 
 *         to configured broker sink if any)
 *         NOTE: This API shall return TRUE if there are no
 *         broker sinks to add to pipeline
 *
 * @param  appCtx [IN]
 * @return TRUE if succussful; FALSE otherwise
 */
static gboolean add_and_link_broker_sink (AppCtx * appCtx);

/** 
 * @brief  Checks if there are any [sink] groups
 *         configured for source_id=provided source_id
 *         NOTE: source_id key and this API is valid only when we
 *         disable [tiler] and thus use demuxer for individual
 *         stream out
 * @param  config [IN] The DS Pipeline configuration struct
 * @param  source_id [IN] Source ID for which a specific [sink]
 *         group is searched for
 */
static gboolean is_sink_available_for_source_id(NvDsConfig *config, guint source_id);

/**
 * Function called at regular interval when one of NV_DS_SOURCE_RTSP type
 * source in the pipeline is down / disconnected. This function try to
 * reconnect the source by resetting that source pipeline.
 */
static gboolean
watch_source_status (gpointer data)
{
  NvDsSrcBin *src_bin = (NvDsSrcBin *) data;

  g_print ("watch_source_status %s\n", GST_ELEMENT_NAME(src_bin));
  if (src_bin && src_bin->reconfiguring) {
    // source is still not up, reconfigure it again.
    g_timeout_add (20, reset_source_pipeline, src_bin);
    return TRUE;
  } else {
    // source is reconfigured, remove call back.
    return FALSE;
  }
}

/**
 * callback function to receive messages from components
 * in the pipeline.
 */
static gboolean
bus_callback (GstBus * bus, GstMessage * message, gpointer data)
{
  AppCtx *appCtx = (AppCtx *) data;
  GST_CAT_DEBUG (NVDS_APP,
      "Received message on bus: source %s, msg_type %s",
      GST_MESSAGE_SRC_NAME (message), GST_MESSAGE_TYPE_NAME (message));
  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_INFO:{
      GError *error = NULL;
      gchar *debuginfo = NULL;
      gst_message_parse_info (message, &error, &debuginfo);
      g_printerr ("INFO from %s: %s\n",
          GST_OBJECT_NAME (message->src), error->message);
      if (debuginfo) {
        g_printerr ("Debug info: %s\n", debuginfo);
      }
      g_error_free (error);
      g_free (debuginfo);
      break;
    }
    case GST_MESSAGE_WARNING:{
      GError *error = NULL;
      gchar *debuginfo = NULL;
      gst_message_parse_warning (message, &error, &debuginfo);
      g_printerr ("WARNING from %s: %s\n",
          GST_OBJECT_NAME (message->src), error->message);
      if (debuginfo) {
        g_printerr ("Debug info: %s\n", debuginfo);
      }
      g_error_free (error);
      g_free (debuginfo);
      break;
    }
    case GST_MESSAGE_ERROR:{
      GError *error = NULL;
      gchar *debuginfo = NULL;
      guint i = 0;
      gst_message_parse_error (message, &error, &debuginfo);
      g_printerr ("ERROR from %s: %s\n",
          GST_OBJECT_NAME (message->src), error->message);
      if (debuginfo) {
        g_printerr ("Debug info: %s\n", debuginfo);
      }

      NvDsSrcParentBin *bin = &appCtx->pipeline.multi_src_bin;
      for (i = 0; i < bin->num_bins; i++) {
        if (bin->sub_bins[i].src_elem == (GstElement *) GST_MESSAGE_SRC (message))
          break;
      }

      if ((i != bin->num_bins) &&
          (appCtx->config.multi_source_config[0].type == NV_DS_SOURCE_RTSP)) {
        // Error from one of RTSP source.
        NvDsSrcBin *subBin = &bin->sub_bins[i];

        if (!subBin->reconfiguring ||
            g_strrstr(debuginfo, "500 (Internal Server Error)")) {
          if (!subBin->reconfiguring) {
            // Check status of stream at regular interval.
            g_timeout_add (SOURCE_RESET_INTERVAL_IN_MS,
                           watch_source_status, subBin);
          }
          // Reconfigure the stream.
          subBin->reconfiguring = TRUE;
          g_timeout_add (20, reset_source_pipeline, subBin);
        }
        g_error_free (error);
        g_free (debuginfo);
        return TRUE;
      }

      g_error_free (error);
      g_free (debuginfo);
      appCtx->return_value = -1;
      appCtx->quit = TRUE;
      break;
    }
    case GST_MESSAGE_STATE_CHANGED:{
      GstState oldstate, newstate;
      gst_message_parse_state_changed (message, &oldstate, &newstate, NULL);
      if (GST_ELEMENT (GST_MESSAGE_SRC (message)) == appCtx->pipeline.pipeline) {
        switch (newstate) {
          case GST_STATE_PLAYING:
            NVGSTDS_INFO_MSG_V ("Pipeline running\n");
            GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (appCtx->
                    pipeline.pipeline), GST_DEBUG_GRAPH_SHOW_ALL,
                "ds-app-playing");
            break;
          case GST_STATE_PAUSED:
            if (oldstate == GST_STATE_PLAYING) {
              NVGSTDS_INFO_MSG_V ("Pipeline paused\n");
            }
            break;
          case GST_STATE_READY:
            GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (appCtx->pipeline.
                    pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "ds-app-ready");
            if (oldstate == GST_STATE_NULL) {
              NVGSTDS_INFO_MSG_V ("Pipeline ready\n");
            } else {
              NVGSTDS_INFO_MSG_V ("Pipeline stopped\n");
            }
            break;
          case GST_STATE_NULL:
            g_mutex_lock (&appCtx->app_lock);
            g_cond_broadcast (&appCtx->app_cond);
            g_mutex_unlock (&appCtx->app_lock);
            break;
          default:
            break;
        }
      }
      break;
    }
    case GST_MESSAGE_EOS:{
      /*
       * In normal scenario, this would use g_main_loop_quit() to exit the
       * loop and release the resources. Since this application might be
       * running multiple pipelines through configuration files, it should wait
       * till all pipelines are done.
       */
      NVGSTDS_INFO_MSG_V ("Received EOS. Exiting ...\n");
      appCtx->quit = TRUE;
      return FALSE;
      break;
    }
    default:
      break;
  }
  return TRUE;
}

static GstBusSyncReply
bus_sync_handler (GstBus * bus, GstMessage * msg, gpointer data)
{
  AppCtx *appCtx = (AppCtx *) data;

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ELEMENT:
      if (GST_MESSAGE_SRC (msg) == GST_OBJECT (appCtx->pipeline.multi_src_bin.bin)) {
        const GstStructure *structure;
        structure = gst_message_get_structure (msg);

       if (gst_structure_has_name (structure, "GstBinForwarded")) {
          GstMessage *child_msg;

          if (gst_structure_has_field (structure, "message")) {
            const GValue *val = gst_structure_get_value (structure, "message");
            if (G_VALUE_TYPE (val) == GST_TYPE_MESSAGE) {
              child_msg = (GstMessage *) g_value_get_boxed (val);
              if (GST_MESSAGE_TYPE(child_msg) == GST_MESSAGE_ASYNC_DONE) {
                guint i = 0;
                NvDsSrcParentBin *bin = &appCtx->pipeline.multi_src_bin;
                GST_DEBUG ("num bins: %d, message src: %s\n", bin->num_bins,
                           GST_MESSAGE_SRC_NAME(child_msg));
                for (i = 0; i < bin->num_bins; i++) {
                  if (bin->sub_bins[i].bin == (GstElement *) GST_MESSAGE_SRC (child_msg))
                    break;
                }

                if (i != bin->num_bins) {
                  NvDsSrcBin *subBin = &bin->sub_bins[i];
                  if (subBin->reconfiguring &&
                      appCtx->config.multi_source_config[0].type == NV_DS_SOURCE_RTSP)
                    g_timeout_add (20, set_source_to_playing, subBin);
                }
              }
            }
          }
        }
      }
      return GST_BUS_PASS;

    default:
      return GST_BUS_PASS;
  }
}

/**
 * Function to dump bounding box data in kitti format. For this to work,
 * property "gie-kitti-output-dir" must be set in configuration file.
 * Data of different sources and frames is dumped in separate file.
 */
static void
write_kitti_output (AppCtx * appCtx, NvDsBatchMeta * batch_meta)
{
  gchar bbox_file[1024] = { 0 };
  FILE *bbox_params_dump_file = NULL;

  if (!appCtx->config.bbox_dir_path)
    return;

  for (NvDsMetaList * l_frame = batch_meta->frame_meta_list; l_frame != NULL;
      l_frame = l_frame->next) {
    NvDsFrameMeta *frame_meta = l_frame->data;
    guint stream_id = frame_meta->pad_index;
    g_snprintf (bbox_file, sizeof (bbox_file) - 1,
        "%s/%02u_%03u_%06lu.txt", appCtx->config.bbox_dir_path,
        appCtx->index, stream_id, (gulong) frame_meta->frame_num);
    bbox_params_dump_file = fopen (bbox_file, "w");
    if (!bbox_params_dump_file)
      continue;

    for (NvDsMetaList * l_obj = frame_meta->obj_meta_list; l_obj != NULL;
        l_obj = l_obj->next) {
      NvDsObjectMeta *obj = (NvDsObjectMeta *) l_obj->data;
      int left = obj->rect_params.left;
      int top = obj->rect_params.top;
      int right = left + obj->rect_params.width;
      int bottom = top + obj->rect_params.height;
      fprintf (bbox_params_dump_file,
          "%s 0.0 0 0.0 %d.00 %d.00 %d.00 %d.00 0.0 0.0 0.0 0.0 0.0 0.0 0.0\n",
          obj->obj_label, left, top, right, bottom);
    }
    fclose (bbox_params_dump_file);
  }
}

/**
 * Function to dump bounding box data in kitti format with tracking ID added.
 * For this to work, property "kitti-track-output-dir" must be set in configuration file.
 * Data of different sources and frames is dumped in separate file.
 */
static void
write_kitti_track_output (AppCtx * appCtx, NvDsBatchMeta * batch_meta)
{
  gchar bbox_file[1024] = { 0 };
  FILE *bbox_params_dump_file = NULL;

  if (!appCtx->config.kitti_track_dir_path)
    return;

  for (NvDsMetaList * l_frame = batch_meta->frame_meta_list; l_frame != NULL;
      l_frame = l_frame->next) {
    NvDsFrameMeta *frame_meta = l_frame->data;
    guint stream_id = frame_meta->pad_index;
    g_snprintf (bbox_file, sizeof (bbox_file) - 1,
        "%s/%02u_%03u_%06lu.txt", appCtx->config.kitti_track_dir_path,
        appCtx->index, stream_id, (gulong) frame_meta->frame_num);
    bbox_params_dump_file = fopen (bbox_file, "w");
    if (!bbox_params_dump_file)
      continue;

    for (NvDsMetaList * l_obj = frame_meta->obj_meta_list; l_obj != NULL;
        l_obj = l_obj->next) {
      NvDsObjectMeta *obj = (NvDsObjectMeta *) l_obj->data;
      int left = obj->rect_params.left;
      int top = obj->rect_params.top;
      int right = left + obj->rect_params.width;
      int bottom = top + obj->rect_params.height;
      guint64 id = obj->object_id;
      fprintf (bbox_params_dump_file,
          "%s %lu 0.0 0 0.0 %d.00 %d.00 %d.00 %d.00 0.0 0.0 0.0 0.0 0.0 0.0 0.0\n",
          obj->obj_label, id, left, top, right, bottom);
    }
    fclose (bbox_params_dump_file);
  }
}

static gint
component_id_compare_func (gconstpointer a, gconstpointer b)
{
  NvDsClassifierMeta *cmetaa = (NvDsClassifierMeta *) a;
  NvDsClassifierMeta *cmetab = (NvDsClassifierMeta *) b;

  if (cmetaa->unique_component_id < cmetab->unique_component_id)
    return -1;
  if (cmetaa->unique_component_id > cmetab->unique_component_id)
    return 1;
  return 0;
}

/**
 * Function to process the attached metadata. This is just for demonstration
 * and can be removed if not required.
 * Here it demonstrates to use bounding boxes of different color and size for
 * different type / class of objects.
 * It also demonstrates how to join the different labels(PGIE + SGIEs)
 * of an object to form a single string.
 */
static void
process_meta (AppCtx * appCtx, NvDsBatchMeta * batch_meta)
{
  // For single source always display text either with demuxer or with tiler
  if (!appCtx->config.tiled_display_config.enable ||
      appCtx->config.num_source_sub_bins == 1) {
    appCtx->show_bbox_text = 1;
  }

  for (NvDsMetaList * l_frame = batch_meta->frame_meta_list; l_frame != NULL;
      l_frame = l_frame->next) {
    NvDsFrameMeta *frame_meta = l_frame->data;
    for (NvDsMetaList * l_obj = frame_meta->obj_meta_list; l_obj != NULL;
        l_obj = l_obj->next) {
      NvDsObjectMeta *obj = (NvDsObjectMeta *) l_obj->data;
      gint class_index = obj->class_id;
      NvDsGieConfig *gie_config = NULL;
      gchar *str_ins_pos = NULL;

      if (obj->unique_component_id ==
          (gint) appCtx->config.primary_gie_config.unique_id) {
        gie_config = &appCtx->config.primary_gie_config;
      } else {
        for (gint i = 0; i < (gint) appCtx->config.num_secondary_gie_sub_bins;
            i++) {
          gie_config = &appCtx->config.secondary_gie_sub_bin_config[i];
          if (obj->unique_component_id == (gint) gie_config->unique_id) {
            break;
          }
          gie_config = NULL;
        }
      }
      g_free (obj->text_params.display_text);
      obj->text_params.display_text = NULL;

      if (gie_config != NULL) {
        if (g_hash_table_contains (gie_config->bbox_border_color_table,
                class_index + (gchar *) NULL)) {
          obj->rect_params.border_color =
              *((NvOSD_ColorParams *)
              g_hash_table_lookup (gie_config->bbox_border_color_table,
                  class_index + (gchar *) NULL));
        } else {
          obj->rect_params.border_color = gie_config->bbox_border_color;
        }
        obj->rect_params.border_width = appCtx->config.osd_config.border_width;

        if (g_hash_table_contains (gie_config->bbox_bg_color_table,
                class_index + (gchar *) NULL)) {
          obj->rect_params.has_bg_color = 1;
          obj->rect_params.bg_color =
              *((NvOSD_ColorParams *)
              g_hash_table_lookup (gie_config->bbox_bg_color_table,
                  class_index + (gchar *) NULL));
        } else {
          obj->rect_params.has_bg_color = 0;
        }
      }

      if (!appCtx->show_bbox_text)
        continue;

      obj->text_params.x_offset = obj->rect_params.left;
      obj->text_params.y_offset = obj->rect_params.top - 30;
      obj->text_params.font_params.font_color =
          appCtx->config.osd_config.text_color;
      obj->text_params.font_params.font_size =
          appCtx->config.osd_config.text_size;
      obj->text_params.font_params.font_name = appCtx->config.osd_config.font;
      if (appCtx->config.osd_config.text_has_bg) {
        obj->text_params.set_bg_clr = 1;
        obj->text_params.text_bg_clr = appCtx->config.osd_config.text_bg_color;
      }

      obj->text_params.display_text = g_malloc (128);
      obj->text_params.display_text[0] = '\0';
      str_ins_pos = obj->text_params.display_text;

      if (obj->obj_label[0] != '\0')
        sprintf (str_ins_pos, "%s", obj->obj_label);
      str_ins_pos += strlen (str_ins_pos);

      if (obj->object_id != UNTRACKED_OBJECT_ID) {
        sprintf (str_ins_pos, " %lu", obj->object_id);
        str_ins_pos += strlen (str_ins_pos);
      }

      obj->classifier_meta_list =
          g_list_sort (obj->classifier_meta_list, component_id_compare_func);
      for (NvDsMetaList * l_class = obj->classifier_meta_list; l_class != NULL;
          l_class = l_class->next) {
        NvDsClassifierMeta *cmeta = (NvDsClassifierMeta *) l_class->data;
        for (NvDsMetaList * l_label = cmeta->label_info_list; l_label != NULL;
            l_label = l_label->next) {
          NvDsLabelInfo *label = (NvDsLabelInfo *) l_label->data;
          if (label->pResult_label) {
            sprintf (str_ins_pos, " %s", label->pResult_label);
          } else if (label->result_label[0] != '\0') {
            sprintf (str_ins_pos, " %s", label->result_label);
          }
          str_ins_pos += strlen (str_ins_pos);
        }

      }
    }
  }
}

/**
 * Function which processes the inferred buffer and its metadata.
 * It also gives opportunity to attach application specific
 * metadata (e.g. clock, analytics output etc.).
 */
static void
process_buffer (GstBuffer * buf, AppCtx * appCtx, guint index)
{
  NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (buf);
  if (!batch_meta) {
    NVGSTDS_WARN_MSG_V ("Batch meta not found for buffer %p", buf);
    return;
  }
  process_meta (appCtx, batch_meta);
  //NvDsInstanceData *data = &appCtx->instance_data[index];
  //guint i;

  //  data->frame_num++;

  /* Opportunity to modify the processed metadata or do analytics based on
   * type of object e.g. maintaining count of particular type of car.
   */
  if (appCtx->all_bbox_generated_cb) {
    appCtx->all_bbox_generated_cb (appCtx, buf, batch_meta, index);
  }
  //data->bbox_list_size = 0;

  /*
   * callback to attach application specific additional metadata.
   */
  if (appCtx->overlay_graphics_cb) {
    appCtx->overlay_graphics_cb (appCtx, buf, batch_meta, index);
  }
}

/**
 * Buffer probe function to get the results of primary infer.
 * Here it demonstrates the use by dumping bounding box coordinates in
 * kitti format.
 */
static GstPadProbeReturn
gie_primary_processing_done_buf_prob (GstPad * pad, GstPadProbeInfo * info,
    gpointer u_data)
{
  GstBuffer *buf = (GstBuffer *) info->data;
  AppCtx *appCtx = (AppCtx *) u_data;
  NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (buf);
  if (!batch_meta) {
    NVGSTDS_WARN_MSG_V ("Batch meta not found for buffer %p", buf);
    return GST_PAD_PROBE_OK;
  }

  write_kitti_output (appCtx, batch_meta);

  return GST_PAD_PROBE_OK;
}

/**
 * Probe function to get results after all inferences(Primary + Secondary)
 * are done. This will be just before OSD or sink (in case OSD is disabled).
 */
static GstPadProbeReturn
gie_processing_done_buf_prob (GstPad * pad, GstPadProbeInfo * info,
    gpointer u_data)
{
  GstBuffer *buf = (GstBuffer *) info->data;
  NvDsInstanceBin *bin = (NvDsInstanceBin *) u_data;
  guint index = bin->index;
  AppCtx *appCtx = bin->appCtx;

  if (gst_buffer_is_writable (buf))
    process_buffer (buf, appCtx, index);
  return GST_PAD_PROBE_OK;
}

/**
 * Buffer probe function after tracker.
 */
static GstPadProbeReturn
analytics_done_buf_prob (GstPad * pad, GstPadProbeInfo * info, gpointer u_data)
{
  NvDsInstanceBin *bin = (NvDsInstanceBin *) u_data;
  guint index = bin->index;
  AppCtx *appCtx = bin->appCtx;
  GstBuffer *buf = (GstBuffer *) info->data;
  NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (buf);
  if (!batch_meta) {
    NVGSTDS_WARN_MSG_V ("Batch meta not found for buffer %p", buf);
    return GST_PAD_PROBE_OK;
  }

  /*
   * Output KITTI labels with tracking ID if configured to do so.
   */
  write_kitti_track_output(appCtx, batch_meta);

  if (appCtx->bbox_generated_post_analytics_cb)
    appCtx->bbox_generated_post_analytics_cb (appCtx, buf, batch_meta, index);
  return GST_PAD_PROBE_OK;
}

static GstPadProbeReturn
latency_measurement_buf_prob(GstPad * pad, GstPadProbeInfo * info, gpointer u_data)
{
  AppCtx *appCtx = (AppCtx *) u_data;
  guint i = 0, num_sources_in_batch = 0;
  if(nvds_enable_latency_measurement)
  {
    GstBuffer *buf = (GstBuffer *) info->data;
    NvDsFrameLatencyInfo *latency_info = NULL;
    g_mutex_lock (&appCtx->latency_lock);
    latency_info = appCtx->latency_info;
    g_print("\n************BATCH-NUM = %d**************\n",batch_num);
    num_sources_in_batch = nvds_measure_buffer_latency(buf, latency_info);

    for(i = 0; i < num_sources_in_batch; i++)
    {
      g_print("Source id = %d Frame_num = %d Frame latency = %lf (ms) \n",
          latency_info[i].source_id,
          latency_info[i].frame_num,
          latency_info[i].latency);
    }
    g_mutex_unlock (&appCtx->latency_lock);
    batch_num++;
  }

  return GST_PAD_PROBE_OK;
}

static gboolean
add_and_link_broker_sink (AppCtx * appCtx)
{
  NvDsConfig *config = &appCtx->config;
  /** Only first instance_bin broker sink 
   * employed as there's only one analytics path for N sources
   * NOTE: There shall be only one [sink] group 
   * with type=6 (NV_DS_SINK_MSG_CONV_BROKER)
   * a) Multiple of them does not make sense as we have only
   * one analytics pipe generating the data for broker sink
   * b) If Multiple broker sinks are configured by the user
   * in config file, only the first in the order of 
   * appearance will be considered
   * and others shall be ignored
   * c) Ideally it should be documented (or obvious) that:
   * multiple [sink] groups with type=6 (NV_DS_SINK_MSG_CONV_BROKER)
   * is invalid
   */
  NvDsInstanceBin *instance_bin = &appCtx->pipeline.instance_bins[0];
  NvDsPipeline *pipeline = &appCtx->pipeline;

  for (guint i = 0; i < config->num_sink_sub_bins; i++) {
    if(config->sink_bin_sub_bin_config[i].type == NV_DS_SINK_MSG_CONV_BROKER)
    {
      if(!pipeline->common_elements.tee) {
         NVGSTDS_ERR_MSG_V ("%s failed; broker added without analytics; check config file\n", __func__);
         return FALSE;
      }
      /** add the broker sink bin to pipeline */
      if(!gst_bin_add (GST_BIN (pipeline->pipeline), instance_bin->sink_bin.sub_bins[i].bin)) {
        return FALSE;
      }
      /** link the broker sink bin to the common_elements tee 
       * (The tee after nvinfer -> tracker (optional) -> sgies (optional) block) */
      if (!link_element_to_tee_src_pad (pipeline->common_elements.tee, instance_bin->sink_bin.sub_bins[i].bin)) {
        return FALSE;
      }
    }
  }
  return TRUE;
}

/**
 * Function to add components to pipeline which are dependent on number
 * of streams. These components work on single buffer. If tiling is being
 * used then single instance will be created otherwise < N > such instances
 * will be created for < N > streams
 */
static gboolean
create_processing_instance (AppCtx * appCtx, guint index)
{
  gboolean ret = FALSE;
  NvDsConfig *config = &appCtx->config;
  NvDsInstanceBin *instance_bin = &appCtx->pipeline.instance_bins[index];
  GstElement *last_elem;
  gchar elem_name[32];

  instance_bin->index = index;
  instance_bin->appCtx = appCtx;

  g_snprintf (elem_name, 32, "processing_bin_%d", index);
  instance_bin->bin = gst_bin_new (elem_name);

  if (!create_sink_bin (config->num_sink_sub_bins,
        config->sink_bin_sub_bin_config, &instance_bin->sink_bin, index)) {
    goto done;
  }

  gst_bin_add (GST_BIN (instance_bin->bin), instance_bin->sink_bin.bin);
  last_elem = instance_bin->sink_bin.bin;

  if (config->osd_config.enable) {
    if (!create_osd_bin (&config->osd_config, &instance_bin->osd_bin)) {
      goto done;
    }

    gst_bin_add (GST_BIN (instance_bin->bin), instance_bin->osd_bin.bin);

    NVGSTDS_LINK_ELEMENT (instance_bin->osd_bin.bin, last_elem);

    last_elem = instance_bin->osd_bin.bin;
  }

  NVGSTDS_BIN_ADD_GHOST_PAD (instance_bin->bin, last_elem, "sink");
  if (config->osd_config.enable) {
    NVGSTDS_ELEM_ADD_PROBE (instance_bin->all_bbox_buffer_probe_id,
        instance_bin->osd_bin.nvosd, "sink",
        gie_processing_done_buf_prob, GST_PAD_PROBE_TYPE_BUFFER, instance_bin);
  } else {
    NVGSTDS_ELEM_ADD_PROBE (instance_bin->all_bbox_buffer_probe_id,
        instance_bin->sink_bin.bin, "sink",
        gie_processing_done_buf_prob, GST_PAD_PROBE_TYPE_BUFFER, instance_bin);
  }

  ret = TRUE;
done:
  if (!ret) {
    NVGSTDS_ERR_MSG_V ("%s failed", __func__);
  }
  return ret;
}

/**
 * Function to create common elements(Primary infer, tracker, secondary infer)
 * of the pipeline. These components operate on muxed data from all the
 * streams. So they are independent of number of streams in the pipeline.
 */
static gboolean
create_common_elements (NvDsConfig * config, NvDsPipeline * pipeline,
    GstElement ** sink_elem, GstElement ** src_elem,
    bbox_generated_callback bbox_generated_post_analytics_cb)
{
  gboolean ret = FALSE;
  *sink_elem = *src_elem = NULL;

  if (config->primary_gie_config.enable) {
    if (config->num_secondary_gie_sub_bins > 0) {
      if (!create_secondary_gie_bin (config->num_secondary_gie_sub_bins,
              config->primary_gie_config.unique_id,
              config->secondary_gie_sub_bin_config,
              &pipeline->common_elements.secondary_gie_bin)) {
        goto done;
      }
      gst_bin_add (GST_BIN (pipeline->pipeline),
          pipeline->common_elements.secondary_gie_bin.bin);
      if (!*src_elem) {
        *src_elem = pipeline->common_elements.secondary_gie_bin.bin;
      }
      if (*sink_elem) {
        NVGSTDS_LINK_ELEMENT (pipeline->common_elements.secondary_gie_bin.bin,
            *sink_elem);
      }
      *sink_elem = pipeline->common_elements.secondary_gie_bin.bin;
    }
  }

  if (config->tracker_config.enable) {
    if (!create_tracking_bin (&config->tracker_config,
            &pipeline->common_elements.tracker_bin)) {
      g_print ("creating tracker bin failed\n");
      goto done;
    }
    gst_bin_add (GST_BIN (pipeline->pipeline),
        pipeline->common_elements.tracker_bin.bin);
    if (!*src_elem) {
      *src_elem = pipeline->common_elements.tracker_bin.bin;
    }
    if (*sink_elem) {
      NVGSTDS_LINK_ELEMENT (pipeline->common_elements.tracker_bin.bin,
          *sink_elem);
    }
    *sink_elem = pipeline->common_elements.tracker_bin.bin;
  }

  if (config->primary_gie_config.enable) {
    if (!create_primary_gie_bin (&config->primary_gie_config,
            &pipeline->common_elements.primary_gie_bin)) {
      goto done;
    }
    gst_bin_add (GST_BIN (pipeline->pipeline),
        pipeline->common_elements.primary_gie_bin.bin);
    if (*sink_elem) {
      NVGSTDS_LINK_ELEMENT (pipeline->common_elements.primary_gie_bin.bin,
          *sink_elem);
    }
    *sink_elem = pipeline->common_elements.primary_gie_bin.bin;
    if (!*src_elem) {
      *src_elem = pipeline->common_elements.primary_gie_bin.bin;
    }
    NVGSTDS_ELEM_ADD_PROBE (pipeline->common_elements.
        primary_bbox_buffer_probe_id,
        pipeline->common_elements.primary_gie_bin.bin, "src",
        gie_primary_processing_done_buf_prob, GST_PAD_PROBE_TYPE_BUFFER,
        pipeline->common_elements.appCtx);
  }

  if(*src_elem) {
    NVGSTDS_ELEM_ADD_PROBE (pipeline->common_elements.
          primary_bbox_buffer_probe_id,
          *src_elem, "src",
          analytics_done_buf_prob, GST_PAD_PROBE_TYPE_BUFFER,
          &pipeline->common_elements);
    pipeline->common_elements.tee = gst_element_factory_make (NVDS_ELEM_TEE, "common_analytics_tee");
    if (!pipeline->common_elements.tee) {
      NVGSTDS_ERR_MSG_V ("Failed to create element 'common_analytics_tee'");
      goto done;
    }

    gst_bin_add (GST_BIN (pipeline->pipeline),
          pipeline->common_elements.tee);

    NVGSTDS_LINK_ELEMENT (*src_elem, pipeline->common_elements.tee);
    *src_elem = pipeline->common_elements.tee;
  }

  ret = TRUE;
done:
  return ret;
}

static gboolean is_sink_available_for_source_id(NvDsConfig *config, guint source_id) {
  for (guint j = 0; j < config->num_sink_sub_bins; j++) {
    if (config->sink_bin_sub_bin_config[j].enable &&
        config->sink_bin_sub_bin_config[j].source_id == source_id) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Main function to create the pipeline.
 */
gboolean
create_pipeline (AppCtx * appCtx,
    bbox_generated_callback bbox_generated_post_analytics_cb,
    bbox_generated_callback all_bbox_generated_cb, perf_callback perf_cb,
    overlay_graphics_callback overlay_graphics_cb)
{
  gboolean ret = FALSE;
  NvDsPipeline *pipeline = &appCtx->pipeline;
  NvDsConfig *config = &appCtx->config;
  GstBus *bus;
  GstElement *last_elem;
  GstElement *tmp_elem1;
  GstElement *tmp_elem2;
  guint i;
  GstPad *fps_pad;
  gulong latency_probe_id;

  _dsmeta_quark = g_quark_from_static_string (NVDS_META_STRING);

  appCtx->all_bbox_generated_cb = all_bbox_generated_cb;
  appCtx->bbox_generated_post_analytics_cb = bbox_generated_post_analytics_cb;
  appCtx->overlay_graphics_cb = overlay_graphics_cb;

  if (config->osd_config.num_out_buffers < 8) {
    config->osd_config.num_out_buffers = 8;
  }

  pipeline->pipeline = gst_pipeline_new ("pipeline");
  if (!pipeline->pipeline) {
    NVGSTDS_ERR_MSG_V ("Failed to create pipeline");
    goto done;
  }

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline->pipeline));
  pipeline->bus_id = gst_bus_add_watch (bus, bus_callback, appCtx);
  gst_bus_set_sync_handler (bus, bus_sync_handler, appCtx, NULL);
  gst_object_unref (bus);

  if (config->file_loop) {
    /* Let each source bin know it needs to loop. */
    guint i;
    for (i = 0; i < config->num_source_sub_bins; i++)
        config->multi_source_config[i].loop = TRUE;
  }

  for (guint i = 0; i < config->num_sink_sub_bins; i++) {
    NvDsSinkSubBinConfig *sink_config = &config->sink_bin_sub_bin_config[i];
    switch (sink_config->type) {
      case NV_DS_SINK_FAKE:
      case NV_DS_SINK_RENDER_EGL:
      case NV_DS_SINK_RENDER_OVERLAY:
        /* Set the "qos" property of sink, if not explicitly specified in the
           config. */
        if (!sink_config->render_config.qos_value_specified) {
          /* QoS events should be generated by sink always in case of live sources
             or with synchronous playback for non-live sources. */
          if (config->streammux_config.live_source || sink_config->render_config.sync) {
            sink_config->render_config.qos = TRUE;
          } else {
            sink_config->render_config.qos = FALSE;
          }
        }
      default:
        break;
    }
  }
  /** Register multi_src_bin specific callbacks if any */
  pipeline->multi_src_bin.rtcp_sender_report_cb = appCtx->rtcp_sender_report_cb;
  /*
   * Add muxer and < N > source components to the pipeline based
   * on the settings in configuration file.
   */
  if (!create_multi_source_bin (config->num_source_sub_bins,
          config->multi_source_config, &pipeline->multi_src_bin))
    goto done;
  gst_bin_add (GST_BIN (pipeline->pipeline), pipeline->multi_src_bin.bin);


  if (config->streammux_config.is_parsed)
    set_streammux_properties (&config->streammux_config,
        pipeline->multi_src_bin.streammux);

  if(appCtx->latency_info == NULL)
  {
    appCtx->latency_info = (NvDsFrameLatencyInfo *)
      calloc(1, config->streammux_config.batch_size *
          sizeof(NvDsFrameLatencyInfo));
  }

  if (config->tiled_display_config.enable) {

    /* Tiler will generate a single composited buffer for all sources. So need
     * to create only one processing instance. */
    if (!create_processing_instance (appCtx, 0)) {
      goto done;
    }
    // create and add tiling component to pipeline.
    if (config->tiled_display_config.columns *
        config->tiled_display_config.rows < config->num_source_sub_bins) {
      if (config->tiled_display_config.columns == 0) {
        config->tiled_display_config.columns =
            (guint) (sqrt (config->num_source_sub_bins) + 0.5);
      }
      config->tiled_display_config.rows =
          (guint) ceil (1.0 * config->num_source_sub_bins /
          config->tiled_display_config.columns);
      NVGSTDS_WARN_MSG_V
          ("Num of Tiles less than number of sources, readjusting to "
          "%u rows, %u columns", config->tiled_display_config.rows,
          config->tiled_display_config.columns);
    }

    gst_bin_add (GST_BIN (pipeline->pipeline), pipeline->instance_bins[0].bin);
    last_elem = pipeline->instance_bins[0].bin;

    if (!create_tiled_display_bin (&config->tiled_display_config,
            &pipeline->tiled_display_bin)) {
      goto done;
    }
    gst_bin_add (GST_BIN (pipeline->pipeline), pipeline->tiled_display_bin.bin);
    NVGSTDS_LINK_ELEMENT (pipeline->tiled_display_bin.bin, last_elem);
    last_elem = pipeline->tiled_display_bin.bin;
  } else {

    /*
     * Create demuxer only if tiled display is disabled.
     */
    pipeline->demuxer =
        gst_element_factory_make (NVDS_ELEM_STREAM_DEMUX, "demuxer");
    if (!pipeline->demuxer) {
      NVGSTDS_ERR_MSG_V ("Failed to create element 'demuxer'");
      goto done;
    }
    gst_bin_add (GST_BIN (pipeline->pipeline), pipeline->demuxer);

    for (i = 0; i < config->num_source_sub_bins; i++) {
      gchar pad_name[16];
      GstPad *demux_src_pad;

      /* Check if any sink has been configured to render/encode output for
       * source index `i`. The processing instance for that source will be
       * created only if atleast one sink has been configured as such.
       */
      if (!is_sink_available_for_source_id(config, i))
        continue;

      if (!create_processing_instance (appCtx, i)) {
        goto done;
      }
      gst_bin_add (GST_BIN (pipeline->pipeline),
          pipeline->instance_bins[i].bin);

      g_snprintf (pad_name, 16, "src_%02d", i);
      demux_src_pad = gst_element_get_request_pad (pipeline->demuxer, pad_name);
      NVGSTDS_LINK_ELEMENT_FULL (pipeline->demuxer, pad_name,
          pipeline->instance_bins[i].bin, "sink");
      gst_object_unref (demux_src_pad);
    }

    last_elem = pipeline->demuxer;
  }
  fps_pad = gst_element_get_static_pad (last_elem, "sink");

  pipeline->common_elements.appCtx = appCtx;
  // Decide where in the pipeline the element should be added and add only if
  // enabled
  if (config->dsexample_config.enable) {
    // Create dsexample element bin and set properties
    if (!create_dsexample_bin (&config->dsexample_config,
            &pipeline->dsexample_bin)) {
      goto done;
    }
    // Add dsexample bin to instance bin
    gst_bin_add (GST_BIN (pipeline->pipeline), pipeline->dsexample_bin.bin);

    // Link this bin to the last element in the bin
    NVGSTDS_LINK_ELEMENT (pipeline->dsexample_bin.bin, last_elem);

    // Set this bin as the last element
    last_elem = pipeline->dsexample_bin.bin;
  }
  // create and add common components to pipeline.
  if (!create_common_elements (config, pipeline, &tmp_elem1, &tmp_elem2,
          bbox_generated_post_analytics_cb)) {
    goto done;
  }

  if(!add_and_link_broker_sink(appCtx)) {
    goto done;
  }

  if (tmp_elem2) {
    NVGSTDS_LINK_ELEMENT (tmp_elem2, last_elem);
    last_elem = tmp_elem1;
  }

  NVGSTDS_LINK_ELEMENT (pipeline->multi_src_bin.bin, last_elem);

  // enable performance measurement and add call back function to receive
  // performance data.
  if (config->enable_perf_measurement) {
    appCtx->perf_struct.context = appCtx;
    enable_perf_measurement (&appCtx->perf_struct, fps_pad,
        pipeline->multi_src_bin.num_bins,
        config->perf_measurement_interval_sec, perf_cb);
  }
  //gst_object_unref (fps_pad);

  NVGSTDS_ELEM_ADD_PROBE (latency_probe_id,
      pipeline->instance_bins->sink_bin.sub_bins[0].sink, "sink",
      latency_measurement_buf_prob, GST_PAD_PROBE_TYPE_BUFFER,
      appCtx);
  latency_probe_id = latency_probe_id;

  GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (appCtx->pipeline.pipeline),
      GST_DEBUG_GRAPH_SHOW_ALL, "ds-app-null");

  g_mutex_init (&appCtx->app_lock);
  g_cond_init (&appCtx->app_cond);
  g_mutex_init (&appCtx->latency_lock);

  ret = TRUE;
done:
  if (!ret) {
    NVGSTDS_ERR_MSG_V ("%s failed", __func__);
  }
  return ret;
}

/**
 * Function to destroy pipeline and release the resources, probes etc.
 */
void
destroy_pipeline (AppCtx * appCtx)
{
  gint64 end_time;
  NvDsConfig *config = &appCtx->config;
  guint i;
  GstBus *bus = NULL;

  end_time = g_get_monotonic_time () + G_TIME_SPAN_SECOND;

  if (!appCtx)
    return;

  if (appCtx->pipeline.demuxer) {
    gst_pad_send_event (gst_element_get_static_pad (appCtx->pipeline.demuxer,
            "sink"), gst_event_new_eos ());
  } else if (appCtx->pipeline.instance_bins[0].sink_bin.bin) {
    gst_pad_send_event (gst_element_get_static_pad (appCtx->
            pipeline.instance_bins[0].sink_bin.bin, "sink"),
        gst_event_new_eos ());
  }

  g_usleep (100000);

  g_mutex_lock (&appCtx->app_lock);
  if (appCtx->pipeline.pipeline) {
    bus = gst_pipeline_get_bus (GST_PIPELINE (appCtx->pipeline.pipeline));

    while (TRUE) {
      GstMessage *message = gst_bus_pop (bus);
      if (message == NULL)
        break;
      else if (GST_MESSAGE_TYPE (message) == GST_MESSAGE_ERROR)
        bus_callback (bus, message, appCtx);
      else
        gst_message_unref (message);
    }
    gst_element_set_state (appCtx->pipeline.pipeline, GST_STATE_NULL);
  }
  g_cond_wait_until (&appCtx->app_cond, &appCtx->app_lock, end_time);
  g_mutex_unlock (&appCtx->app_lock);

  for (i = 0; i < appCtx->config.num_source_sub_bins; i++) {
    NvDsInstanceBin *bin = &appCtx->pipeline.instance_bins[i];
    if (config->osd_config.enable) {
      NVGSTDS_ELEM_REMOVE_PROBE (bin->all_bbox_buffer_probe_id,
          bin->osd_bin.nvosd, "sink");
    } else {
      NVGSTDS_ELEM_REMOVE_PROBE (bin->all_bbox_buffer_probe_id,
          bin->sink_bin.bin, "sink");
    }

    if (config->primary_gie_config.enable) {
      NVGSTDS_ELEM_REMOVE_PROBE (bin->primary_bbox_buffer_probe_id,
          bin->primary_gie_bin.bin, "src");
    }

  }
  if(appCtx->latency_info == NULL)
  {
    free(appCtx->latency_info);
    appCtx->latency_info = NULL;
  }

  g_mutex_clear(&appCtx->latency_lock);

  if (appCtx->pipeline.pipeline) {
    bus = gst_pipeline_get_bus (GST_PIPELINE (appCtx->pipeline.pipeline));
    gst_bus_remove_watch (bus);
    gst_object_unref (bus);
    gst_object_unref (appCtx->pipeline.pipeline);
  }
}

gboolean
pause_pipeline (AppCtx * appCtx)
{
  GstState cur;
  GstState pending;
  GstStateChangeReturn ret;
  GstClockTime timeout = 5 * GST_SECOND / 1000;

  ret =
      gst_element_get_state (appCtx->pipeline.pipeline, &cur, &pending,
      timeout);

  if (ret == GST_STATE_CHANGE_ASYNC) {
    return FALSE;
  }

  if (cur == GST_STATE_PAUSED) {
    return TRUE;
  } else if (cur == GST_STATE_PLAYING) {
    gst_element_set_state (appCtx->pipeline.pipeline, GST_STATE_PAUSED);
    gst_element_get_state (appCtx->pipeline.pipeline, &cur, &pending,
        GST_CLOCK_TIME_NONE);
    pause_perf_measurement (&appCtx->perf_struct);
    return TRUE;
  } else {
    return FALSE;
  }
}

gboolean
resume_pipeline (AppCtx * appCtx)
{
  GstState cur;
  GstState pending;
  GstStateChangeReturn ret;
  GstClockTime timeout = 5 * GST_SECOND / 1000;

  ret =
      gst_element_get_state (appCtx->pipeline.pipeline, &cur, &pending,
      timeout);

  if (ret == GST_STATE_CHANGE_ASYNC) {
    return FALSE;
  }

  if (cur == GST_STATE_PLAYING) {
    return TRUE;
  } else if (cur == GST_STATE_PAUSED) {
    gst_element_set_state (appCtx->pipeline.pipeline, GST_STATE_PLAYING);
    gst_element_get_state (appCtx->pipeline.pipeline, &cur, &pending,
        GST_CLOCK_TIME_NONE);
    resume_perf_measurement (&appCtx->perf_struct);
    return TRUE;
  } else {
    return FALSE;
  }
}
