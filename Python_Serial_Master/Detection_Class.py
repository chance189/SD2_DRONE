'''
* Author: Chance Reimer
* Purpose: Generate useful wrapper for keeping track of stats for a certain tracked target
*          The unique ID is given by the Deepstream API via a certain message, that will notify main thread to create this object. 
* Note: this object will handle all information for its unique ID
'''

import os
import traceback
import math
from collections import deque
from detections import detections

class Tracked_Object:
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
            (x1, y1) = (detections[0].get_X(), detections[0].get_Y())  #push from right, so
            (x2, y2) = (detections[1].get_X(), detections[1].get_Y())  #index 1 is most recent
            dist = math.sqrt((x2-x1)**2 + (y2-y1)**2)                  #Distance formula
            this.velocity = dist/(detections[1].get_timeStamp() - detections[0].get_timeStamp())

    #Using center of screen as reference point (size of camera is defined, so it will be 1/2 x, 1/2 y for coordinates
    #Need to find distance from center, and calculate it using atan2
    def calc_theta(self):
        if len(self.detections) == 2:
            self.theta = -1
