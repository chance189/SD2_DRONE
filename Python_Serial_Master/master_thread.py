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

class master_thread:
    def __init__(self):
        #init all queues (remember they are thread safe in python)
        self.recv_q = queue.Queue()
        self.tx_q = queue.Queue()
        self.meta_data_pipe = queue.Queue()
        #init and start our threads
        self.init_threads()

    def init_threads(self):
        self.serial_thread = serial_thread(recv_queue=self.recv_q, tx_queue=self.tx_q)
        self.socket_thread = deepstream_socket_thread(info_pipe=self.meta_data_pipe)
        self.serial_thread.start()
        self.socket_thread.start()
    
    def run_main_prog(self):
        while(True):
            #do stuff
            pass

    def handle_new_metadata(self):
        return 1
