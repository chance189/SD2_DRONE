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

NACK = b'~'  #simple 1 byte agreement that isn't used in my debug statements

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
        self.print_locker = lock      #thread safe writes
        self.ts_print("Serial Port Init")
    
    def run(self):
        self.ts_print("in Serial Thread")
        while(True):
            if self.serial_conn.in_waiting > 0:
                rx = self.serial_conn.read(1)                        #expect a 1 byte word
                if rx == NACK:
                    self.ts_print("NACK RX!")
                    if self.serial_conn.out_waiting != 0:
                        self.ts_print("{0} bytew were in output buffer! Clearing!".format(serial_conn.out_waiting))
                    self.serial_conn.reset_output_buffer()    #reset the output
                self.recv_q.put(rx)

            if not self.tx_q.empty():
                transmit = self.tx_q.get();
                transmit += self.crc8(transmit)
                #transmit = (int(0x7F).to_bytes(1, byteorder="little", signed="False")) + transmit #add start byte
                self.ts_print("We sent: {0}, #b: {1}, at {2}".format(transmit, len(transmit), time.time()))
                self.serial_conn.write(transmit)

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

    #input of a bytearray
    def crc8(self, bytes_in, poly=0xE0):
        crc = 0xFF
        for byte in bytes_in:
            cur_byte = 0xFF & byte
            for _ in range(0, 8):
                if (crc & 0x01) ^ (cur_byte & 0x01):
                    crc = (crc >> 1) ^ poly
                else:
                    crc >>= 1
                cur_byte >>= 1
        return (crc & 0xFF).to_bytes(1, byteorder="little", signed=False)
    
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

