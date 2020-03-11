'''
# Author: Chance Reimer
# Purpose: to create a timer thread that will obtain mutex and make a variable true in order to
#          enable sending to the slave board again
'''

import threading
import time
import sys
import traceback
import master_thread

class Timer_Mod(threading.Thread):
    def __init__(self, masterclass_var):
        threading.Thread.__init__(self)
        self.master_class_ref = masterclass_var
        self.running = False
        with self.master_class_ref.locker:
            print("INIT TIMER!")

    '''
    Timeout module, make it so that we can send data again
    '''
    def run(self):
        self.running = True
        time.sleep(0.1)
        with self.master_class_ref.locker:
            self.master_class_ref.sent_data = False
            print("WAITING FINISHED!")
        self.running = False

    #make sure to run join after calling this function
    def sepuku(self):
        thread_id = self.get_id()
        result = ctypes.pythonapi.PyThreadState_SetAsyncExc(thread_id, ctypes.py_object(SystemExit))
        if result > 1:
            ctypes.pythonapi.PyThreadState_SetAsyncExc(thread_id, 0)
            print("Attempt to Raise Exception Failed:\nSource: Deepstream_Socket\nDiagnosis: FATAL")
