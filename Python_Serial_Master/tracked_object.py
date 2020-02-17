'''
* Author: Chance Reimer
* Purpose: Generate useful wrapper for keeping track of stats for a certain tracked target
*          The unique ID is given by the Deepstream API via a certain message, that will notify main thread to create this object. 
* Note: this object will handle all information for its unique ID
'''

import os
import traceback
import math
import time
from collections import deque
from detections import detections
from math import *
from threading import Lock

centerX = 1920/2   #this is our width of screen divided by 2
centerY = 1080/2 

class tracked_object:
    def __init__(self, unique_id=1):
        self.detections = deque(maxlen=2)  #create last two detections, FIFO
        self.velocity = -1.0
        self.distance = -1.0
        self.theta = -1.0
        self.unique_id = unique_id
        self.locker = Lock()               #Mutex for ensuring threadsafe prints

    def grab_velocity(self):
        return self.velocity

    def grab_theta(self):
        return self.theta
    
    def ts_print(self, *a, **b):
        with self.locker:
            print(*a, **b)

    def update_coordinates(self, detection):
        if detection.get_ID() != self.unique_id:
            self.ts_print("Invalid coordinate for current Obj")
        else:
            self.detections.append(detection)
            self.calc_velocity()
            self.calc_theta()
            if self.locker:
                self.ts_print("Updated Stats for ID: {0}\nVelocity(ft/s): {1}\nAngle (degrees) {2}".format(self.unique_id, self.velocity, self.theta))

    def calc_velocity(self):
        if len(self.detections) == 2:
            (x1, y1) = (self.detections[0].get_X(), self.detections[0].get_Y())  #push from right, so
            (x2, y2) = (self.detections[1].get_X(), self.detections[1].get_Y())  #index 1 is most recent
            dist = sqrt((x2-x1)**2 + (y2-y1)**2)                  #Distance formula
            self.velocity = dist/(self.detections[1].get_timeStamp() - self.detections[0].get_timeStamp())

    #Using center of screen as reference point (size of camera is defined, so it will be 1/2 x, 1/2 y for coordinates
    #Need to find distance from center, and calculate it using atan2
    def calc_theta(self):
        if len(self.detections) == 2:
            self.x_mod_coord = self.detections[1].get_X() - centerX
            self.y_mod_coord = self.detections[1].get_Y() - centerY
            self.theta = degrees(math.atan2(self.y_mod_coord, self.x_mod_coord))

    def grab_relevant_data(self):
        return (self.grab_velocity(), self.grab_theta())

    def package_serial(self):
        #Send one byte for the panning, and one for the tilt, signed 8 bit number
        byte_X = (int(self.y_mod_coord/tan(radians(self.theta)))%128).to_bytes(1, byteorder="little", signed=True)
        byte_Y = (int(tan(radians(self.theta))*self.x_mod_coord)%128).to_bytes(1, byteorder="little", signed=True)
        bytes_to_send = bytearray()
        bytes_to_send += bytearray(byte_X)
        bytes_to_send += bytearray(byte_Y)
        return bytes_to_send
