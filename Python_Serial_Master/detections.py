'''
Author: Chance  Reimer
Purpose: simple class for keeping track of a detection
Future Updates: There is simple support for target ids/multiple targets, but it just defaults to 1
'''

class detections:
    def __init__(self, x=None, y=None, h=None, w=None, label=None, unique_ID=1, timeStamp=None):
        self.x = x
        self.y = y
        self.h = h
        self.w = w
        self.label = label
        self.unique_ID = unique_ID
	self.timeStamp = timeStamp

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
