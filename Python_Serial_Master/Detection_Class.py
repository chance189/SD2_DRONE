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
            this.velocity = dist/(detections[1].get_timeStamp() - detections[0].get_timeStamp()


