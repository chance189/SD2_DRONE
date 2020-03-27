'''
* Author: Chance Reimer
* Modified from: stackoverflow.com/questions/4440439/pyqt-showing-video-stream-from-opencv
* Purpose: Display video feed and printout results from the deepstream sdk
'''

from PyQt5.QtWidgets import QWidget, QLabel, QApplication
from PyQt5.QtCore import QThread, Qt, pyqtSignal, pyqtSlot
from PyQt5.QtGui import QImage, QPixmap
import numpy
from gstreamer import GstContext, GstPipeline, GstApp, Gst, GstVideo

class Thread(QThread):
    changePixmap = pyqtSignal(QImage)
    
    def setup_gstream(self):
        self.command_work_too_slow = "gst-launch-1.0 v4l2src device=/dev/video0 ! image/jpeg, width=1920, height=1080, framerate=60/1 ! jpegparse ! jpegdec ! x264enc ! h264parse ! nvv4l2decoder ! m.sink_0 nvstreammux name=m batch-size=1 width=1920 height=1080 ! nvinfer config-file-path=/home/chance/Desktop/SD2_DRONE/deepstream_proof_using_config/config_infer_primary_yoloV3_tiny.txt batch-size=1 unique-id=1 ! nvvideoconvert ! nvdsosd ! nvegltransform ! nveglglessink"
        self.command_still_to_slow = "gst-launch-1.0 v4l2src device=/dev/video0 ! image/jpeg, width=1280, height=720, framerate=60/1 ! jpegparse ! nvv4l2decoder ! m.sink_0 nvstreammux name=m batch-size=1 width=1280 height=720 ! nvinfer config-file-path=/home/chance/Desktop/SD2_DRONE/deepstream_proof_using_config/config_infer_primary_yoloV3_tiny.txt batch-size=1 unique-id=1 ! nvvideoconvert ! nvdsosd ! nvegltransform ! nveglglessink"
        self.try3 = "gst-launch-1.0 v4l2src device=/dev/video0 ! image/jpeg, width=1280, height=720, framerate=60/1 ! jpegparse ! nvv4l2decoder ! m.sink_0 nvstreammux name=m batch-size=1 width=1280 height=720 gpu-id=0 nvbuf-memory-type=0 ! nvinfer config-file-path=/home/chance/Desktop/SD2_DRONE/deepstream_proof_using_config/config_infer_primary_yoloV3_tiny.txt batch-size=1 unique-id=1 ! queue ! nvvideoconvert ! nvdsosd ! nvegltransform ! nveglglessink"

    def run(self):


class App(QWidget):

    def __init__(self):
        super().__init__()
        self.initUI()

    @pyqtSlot(QImage)
    def setImage(self, image):
        self.label.setPixmap(QPixmap.fromImage(image))

    def initUI(self):
        self.setWIndowTitle(self.title)
        self.setGeometry(self.left, self.top, self.width, self.height)
        self.resize(1800, 1200)

        #create label
        self.label = QLabel(self)
        self.label.move(280, 120)
        self.label.resize(640, 480)
        th = Thread(self)
        th.changePixmap.connect(self.setImage)
        th.start()
        self.show()
