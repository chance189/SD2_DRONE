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

centerX = 1366/2  - 50  #this is our width of screen divided by 2
centerY = 768/2   - 70
width_drone = 3.9  #size of drone in inches

class tracked_object:
    def __init__(self, locker,  unique_id=1):
        self.detections = deque(maxlen=2)  #create last two detections, FIFO
        self.velocity = -1.0
        self.distance = -1.0
        self.theta = -1.0
        self.unique_id = unique_id
        self.locker = locker               #Mutex for ensuring threadsafe prints
        
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
            self.servo_degree_calc()
            #if self.locker:
                #self.ts_print("Updated Stats for ID: {0}\nVelocity(ft/s): {1}\nAngle (degrees) {2}\nDistance: {3}".format(self.unique_id, self.velocity, self.theta, self.detections[-1].get_dist()))

    def calc_velocity(self):
        if len(self.detections) == 2:
            (x1, y1) = (self.detections[0].get_center_coord())  #push from right, so
            (x2, y2) = (self.detections[1].get_center_coord())  #index 1 is most recent
            inch_to_pixel = width_drone/((self.detections[0].get_W() + self.detections[1].get_W())/2)
            dist = sqrt((x2-x1)**2 + (y2-y1)**2)*inch_to_pixel  #Distance formula, convert to inches
            self.velocity = dist/(self.detections[1].get_timeStamp() - self.detections[0].get_timeStamp())

    #Using center of screen as reference point (size of camera is defined, so it will be 1/2 x, 1/2 y for coordinates
    #Need to find distance from center, and calculate it using atan2
    def calc_theta(self):
        (x_c, y_c) = self.detections[-1].get_center_coord()
        self.x_mod_coord = x_c - centerX
        #because of setup, neg values go down, pos values go up
        #but, y is positive going down, meaning if centerY - y_c is neg, we should go down
        self.y_mod_coord = centerY - y_c   

        if len(self.detections) == 2:
            self.theta = degrees(math.atan2(self.y_mod_coord, self.x_mod_coord))
    
    def servo_degree_calc(self):
        inch_to_pixel = width_drone/self.detections[-1].get_W()
        self.servo_X_delta = (int)(2.5*degrees(atan2((self.x_mod_coord*inch_to_pixel), self.detections[-1].get_dist())))
        self.servo_Y_delta = (int)(2.5*degrees(atan2((self.y_mod_coord*inch_to_pixel), self.detections[-1].get_dist())))
        #print("Byte X: {0}, Byte Y: {1}".format(self.servo_X_delta, self.servo_Y_delta))
        #print("x_mod_coord: {0}, y_mod_coord: {1}".format(self.x_mod_coord, self.y_mod_coord))
        #print("Distance: {0}, Width in Bytes: {1}, ratio: {2}".format(self.detections[-1].get_dist(), self.detections[-1].get_W(), inch_to_pixel))

    def grab_relevant_data(self):
        return (self.servo_X_delta, self.servo_Y_delta, self.grab_velocity(), self.grab_theta())

    def package_serial(self):
        #Send one byte for the panning, and one for the tilt, signed 8 bit number
        #want to catch the exceptions that we divide by 0
        try:
            byte_X = (self.servo_X_delta).to_bytes(1, byteorder="little", signed=True)
            byte_Y = (self.servo_Y_delta).to_bytes(1, byteorder="little", signed=True)
        except Exception as e:
            print(e)
            byte_X = (1).to_bytes(1, byteorder="little", signed=True)
            byte_Y = (1).to_bytes(1, byteorder="little", signed=True)
        print("Byte X: {0}, Byte Y: {1}".format(self.servo_X_delta, self.servo_Y_delta))
        print("x_mod_coord: {0}, y_mod_coord: {1}".format(self.x_mod_coord, self.y_mod_coord))
        bytes_to_send = bytearray()
        bytes_to_send += byte_X
        bytes_to_send += byte_Y
        #bytes_to_send = (127).to_bytes(1, byteorder="little", signed=True) + bytes_to_send
        return bytes_to_send
