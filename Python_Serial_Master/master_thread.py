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
from threading import RLock
import sys
import time
import RPi.GPIO as GPIO
import math
from PyQt5.QtCore import QThread, Qt, pyqtSignal, pyqtSlot

class master_thread(QThread):
    #Oh lordy look at all those signals
    change_vel = pyqtSignal(float)
    change_dist = pyqtSignal(float)
    change_status = pyqtSignal('QString')
    def __init__(self):
        QThread.__init__(self)
        #init all queues (remember they are thread safe in python)
        self.recv_q = queue.Queue()
        self.tx_q = queue.Queue()
        self.meta_data_pipe = queue.Queue()
        #init and start our threads
        self.locker = RLock()
        self.init_threads()
        self.tracked_objs = {}
        self.timer = threading.Timer(1.5, self.timeout_rst)
        self.sent_data = False          #used for ensuring that
       
        #Note to self, GPIO does not yield high enough voltage to drive our laser
        self.fire_timer = threading.Timer(1, self.timeout_fire)
        self.status = "IDLE"

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
    
    def run(self):
        self.ts_print("In main thread")
        while(True):
            #do stuff
            #print("HearBeat Test: MainThread")
            self.handle_new_metadata()
            self.handle_new_arduino_msg()

    def ts_print(self, *a, **b):
        with self.locker:
            print(*a, **b)

    def fire_laser(self):
        if self.fire_timer.isAlive():
            self.fire_timer.cancel()
        self.fire_timer = threading.Timer(1, self.timeout_fire)
        self.fire_timer.start()
        with self.locker:
            self.status = "FIRING"
            self.change_status.emit(self.status)

    def timeout_fire(self):
        with self.locker:
            self.status = "IDLE"
            self.change_status.emit(self.status)

    def timeout_rst(self):
        self.ts_print("Timeout Occurred!")
        with self.locker:
            self.sent_data = False

    def handle_new_metadata(self):
        if not self.meta_data_pipe.empty():
            #self.ts_print("Handling new Metadata")
            data = self.meta_data_pipe.get()
            if data.get_ID() in self.tracked_objs:
                self.tracked_objs[data.get_ID()].update_coordinates(data)
                #(x_c, y_c) = data.get_center_coord()
                #self.ts_print("The x center: {0}, the y center: {1}, sent_data: {2}".format(x_c, y_c, self.sent_data))
                #self.ts_print("Added to ID: {0}".format(data.get_ID()))
            else:
                #self.ts_print("Adding new ID: {0} to dictionary".format(data.get_ID()))
                self.tracked_objs[data.get_ID()] = tracked_object.tracked_object(unique_id=data.get_ID(), locker=self.locker)
                self.tracked_objs[data.get_ID()].update_coordinates(data)
            
            (servo_X, servo_Y, velocity, theta) = self.tracked_objs[data.get_ID()].grab_relevant_data()
            self.change_vel.emit(velocity)
            self.change_dist.emit(data.get_dist())
            
            with self.locker:
                if self.status == "IDLE":
                    self.status = "TRACKING"
                    self.change_status.emit(self.status)

            if math.fabs(servo_X) <= 2 and math.fabs(servo_Y) <= 2:
                self.fire_laser()

            
            #Here lies my madness with dumb fucking timers
            if self.sent_data:
                pass
            else:
                self.tx_q.put(self.tracked_objs[data.get_ID()].package_serial())
                
                with self.locker:
                    self.sent_data = True
                
                if not self.timer.isAlive():
                    try:
                        self.timer = threading.Timer(1.5, self.timeout_rst)
                        self.timer.start()
                    except Exception as e:
                        self.ts_print("Attempting to join!!!")
                        self.timer.join()
                        self.timer.start()
                        self.ts_print("Successful joining!")
            
    def handle_new_arduino_msg(self):
        if not self.recv_q.empty():
            byte_string = self.recv_q.get()
            if byte_string == b'*':
                if self.timer.isAlive():   #only perform action if timer is active
                    self.timer.cancel()    #received reply b4 timeout, kill the thread
                    self.ts_print("Timer Killed")
                    with self.locker:
                        self.sent_data = False
                self.ts_print("Arduino sent bytestring: {0}".format(byte_string))

'''
if __name__ == "__main__":
    run_prog = master_thread()
    run_prog.run_main_prog()
    GPIO.cleanup()
'''
