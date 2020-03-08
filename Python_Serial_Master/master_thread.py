'''
* Author: Chance Reimer
* Purpose: Master Control Thread, starts all other threads and manages their information
* Note: this will most likely be where the GUI goes too
'''

import queue
import threading
import detections
import deepstream_socket_thread
import serial_thread
import detect_pipe_pb2
import tracked_object
import traceback
from threading import Lock

class master_thread:
    def __init__(self):
        #init all queues (remember they are thread safe in python)
        self.recv_q = queue.Queue()
        self.tx_q = queue.Queue()
        self.meta_data_pipe = queue.Queue()
        #init and start our threads
        self.locker = Lock()
        self.init_threads()
        self.tracked_objs = {}

    def init_threads(self):
        self.my_serial_thread = serial_thread.serial_thread(recv_queue=self.recv_q, tx_queue=self.tx_q, lock=self.locker)
        self.socket_thread = deepstream_socket_thread.deepstream_socket_thread(info_pipe=self.meta_data_pipe, lock=self.locker)
        try:
            self.my_serial_thread.start()
        except Exception as e:
            self.ts_print("Exception in serial_thread:")
            traceback.print_exc(file=sys.stdout)
        try:
            self.socket_thread.start()
        except Exception as e:
            self.ts_print("Exception in socket thread")
            traceback.print_exc(file=sys.stdout)
    
    def run_main_prog(self):
        self.ts_print("In main thread")
        while(True):
            #do stuff
            #print("HearBeat Test: MainThread")
            self.handle_new_metadata()
            self.handle_new_arduino_msg()

    def ts_print(self, *a, **b):
        with self.locker:
            print(*a, **b)

    def handle_new_metadata(self):
        if not self.meta_data_pipe.empty():
            self.ts_print("Handling new Metadata")
            data = self.meta_data_pipe.get()
            if data.get_ID() in self.tracked_objs:
                self.tracked_objs[data.get_ID()].update_coordinates(data)
                (x_c, y_c) = data.get_center_coord()
                self.ts_print("The x center: {0}, the y center: {1}".format(x_c, y_c))
                #self.ts_print("Added to ID: {0}".format(data.get_ID()))
            else:
                #self.ts_print("Adding new ID: {0} to dictionary".format(data.get_ID()))
                self.tracked_objs[data.get_ID()] = tracked_object.tracked_object(data.get_ID())
                self.tracked_objs[data.get_ID()].update_coordinates(data)
            
            (velocity, theta) = self.tracked_objs[data.get_ID()].grab_relevant_data()
            if velocity < 1:
                pass
            else:
                self.tx_q.put(self.tracked_objs[data.get_ID()].package_serial())
    
    def handle_new_arduino_msg(self):
        if not self.recv_q.empty:
            byte_string = self.recv_q.get()
            #self.ts_print("Arduino sent bytestring: {0}".format(byte_string))

if __name__ == "__main__":
    run_prog = master_thread()
    run_prog.run_main_prog()
