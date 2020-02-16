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

centerX = 1920/2   #this is our width of screen divided by 2
centerY = 1080/2 

class tracked_object:
    def __init__(self, unique_id=1):
        self.detections = deque(maxlen=2)  #create last two detections, FIFO
        self.velocity = -1.0
        self.distance = -1.0
        self.theta = -1.0
        self.unique_id = unique_id

    def grab_velocity(self):
        return self.velocity

    def grab_theta(self):
        return self.theta

    def update_coordinates(self, detection):
        if detection.get_ID() != this.unique_id:
            print("Invalid coordinate for current Obj")
        else:
            self.detections.append(detection)
            self.calc_velocity()
            self.calc_theta()

    def calc_velocity(self):
        if len(self.detections) == 2:
            (x1, y1) = (self.detections[0].get_X(), self.detections[0].get_Y())  #push from right, so
            (x2, y2) = (self.detections[1].get_X(), self.detections[1].get_Y())  #index 1 is most recent
            dist = sqrt((x2-x1)**2 + (y2-y1)**2)                  #Distance formula
            this.velocity = dist/(self.detections[1].get_timeStamp() - self.detections[0].get_timeStamp())

    #Using center of screen as reference point (size of camera is defined, so it will be 1/2 x, 1/2 y for coordinates
    #Need to find distance from center, and calculate it using atan2
    def calc_theta(self):
        if len(self.detections) == 2:
            self.x_mod_coord = self.detections[1].get_X() - centerX
            self.y_mod_coord = self.detections[1].get_Y() - centerY
            self.theta = degrees(math.atan2(y_mod_coord, x_mod_coord))

    def grab_relevant_data(self):
        return (self.grab_velocity(), self.grab_theta)

    def package_serial(self):
        #Send one byte for the panning, and one for the tilt, signed 8 bit number
        byte_X = int(self.y_mod_coord/tan(radians(self.theta))).to_bytes(1, byteorder="little", signed=True)
        byte_Y = int(tan(radians(self.theta))*self.x_mod_coord).to_bytes(1, byteorder="little", signed=True)
        bytes_to_send = bytearray()
        bytes_to_send.append(byte_X)
        bytes_to_send.append(byte_Y)
        return bytes_to_send

        
