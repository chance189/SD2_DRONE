'''
Author: Chance  Reimer
Purpose: simple class for keeping track of a detection
Future Updates: There is simple support for target ids/multiple targets, but it just defaults to 1
'''
import time

'''
* Note for distance measurement:
* Refer to equation:
    F = (P*D)/W  # this is F - Focal Length, P - pixels, D - distance, W - width of obj
    P = 250 (measured), W = 3.9in, D = 24in
    F = (3.9*250)/24 = 40.625
'''
focal_length = 1569.231
width = 3.9                 #in inches

class detections:
    def __init__(self, x=None, y=None, h=None, w=None, label=None, unique_ID=1, timeStamp=None):
        self.h = h
        self.w = w
        self.x = x                      #grab center x
        self.y = y                      #grab center y
        self.label = label
        self.unique_ID = unique_ID
        self.timeStamp = timeStamp
        self.calc_dist()

    '''
    * D = (W*F)/P
    * find distance in units given, which is inches
    * D is what we want to solve for,
    * W = width of drone (3.9 in), F is focal length, precalculated, P is pixel width (self.w)
    '''
    def calc_dist(self):
        self.dist = (width*focal_length)/self.w

    def get_X(self):
        return self.x

    def get_Y(self):
        return self.y

    def get_H(self):
        return self.h

    def get_W(self):
        return self.w

    def get_label(self):
        return self.label

    def get_ID(self):
        return self.unique_ID
    
    def get_timeStamp(self):
        return self.timeStamp

    def get_dist(self):
        return self.dist

    #returns the actual center x, center y, approximate size of drone
    def get_center_coord(self):  
        x_c = self.x + 0.5*self.w
        y_c = self.y + 0.5*self.h

        return (x_c, y_c)
