'''
* Author: Chance Reimer
* Purpose: Creates a thread to manage all serial communications, will send any interrupt based information from the arduino,
*          and polls the queue that is tied to it for any updated commands to send down to arduino
* Note: CRC8 communication will be calculated on this thread with the information sent by the main thread
'''
import queue
import serial
import threading
from threading import RLock
import traceback
import time
import ctypes

NACK = bytearray("INVALIDMSG", 'utf-8')  #Want 40 bit, so 10 char (account for CRC)

class serial_thread(threading.Thread):
    #Sanity check here
    #tx_queue will hold items that are SENT to Arduino
    #recv_queue will hold items that are RECEIVED from arduino
    #Both queues will hold binary arrays
    def __init__(self, recv_queue, tx_queue, lock):
        threading.Thread.__init__(self)
        self.recv_q = recv_queue
        self.tx_q = tx_queue
        self.open_serial_port()
        self.lms = None                 #Last message sent
        self.print_locker = lock      #thread safe writes
        self.ts_print("Serial Port Init")
    
    def run(self):
        self.ts_print("in Serial Thread")
        while(True):
            if not self.tx_q.empty():
                transmit = self.tx_q.get();
                self.ts_print("We sent: {0}, at {1}".format(transmit, time.time()))
                #transmit.append(self.calc_CRC8(transmit))
                self.serial_conn.write(transmit)
                #self.serial_conn.write(transmit[0].encode('utf8'))
                #self.serial_conn.write(transmit[1].encode('utf8'))
                self.lms = transmit
            if self.serial_conn.in_waiting > 0:
                rx = self.serial_conn.read(1)                        #expect a 2 byte word
                if rx == NACK:
                    self.write(lms)
                self.recv_q.put(rx)
                #elif self.calc_CRC8 != rx[4]:       #check CRC
                    #self.write(NACK)                #Notify Arduino CRC mismatch

    def ts_print(self, *a, **b):   #threadsafe way to print data
        with self.print_locker:
            print(*a, **b)

    def open_serial_port(self):
        try:
            self.serial_conn = serial.Serial()
            self.serial_conn.port = "/dev/ttyTHS1"
            self.serial_conn.baudrate = 9600
            self.serial_conn.timeout = 1
            self.serial_conn.setDTR(False)
            self.serial_conn.setRTS(False)
            self.serial_conn.open()
        except Exception as e:
            print("Serial Connection Failed")
            print(traceback.format_exc())

    def calc_CRC8(inByteString, CRC):
        return "This isn't done yet"
    
    def get_id(self):
        if hasattr(self, '_thread_id'):
            return self._thread_id

        for id, thread in threading._active.items():
            if thread is self:
                return id
    
    #remember to join after calling this function
    def kill_the_traitor(self):
        thread_id = self.get_id()
        res = ctypes.pythonapi.PYThreadState_SetAsyncExc(thread_id, ctypes.py_object(SystemExit))
        if res > 1:
            ctypes.pythonapi.PyThreadState_SetAsyncExc(thread_id, 0)
            print("Failed to kill thread: Serial")

