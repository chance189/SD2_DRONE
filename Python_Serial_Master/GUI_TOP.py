'''
# Author: Chance Reimer
# Purpose: To create a GUI that will show status of the drone
# Note: I'm so sorry.
'''
from GUI_UI import *  #because why select what you need? We are american after all, TAKE IT ALL
from PyQt5.QtWidgets import QWidget, QLabel, QApplication, QMainWindow, QVBoxLayout, QSizePolicy, QPlainTextEdit
from PyQt5.QtCore import QThread, Qt, pyqtSignal, pyqtSlot
from PyQt5.QtGui import QImage, QPixmap, QWindow, QTextCursor
from master_thread import master_thread
import sys
import time

class GUI_TOP(QMainWindow):

    def __init__(self):
        super().__init__()
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        #Tie action socket for toggling menu

        '''
        **** Here lies my great shame ****
        So we need the window ID, however, DeepStream SDK is a naughty boy
        and he doesn't give us a PID associated with the window. I'm too
        lazy to come up with a good solution, so here is the bad one:

        ensure you have wmctrl, by sudo apt-get install wmctrl (assuming debian)

        next run wmctrl -lp to get a list of all open windows. One of them will 
        have a listing similar to below:
        (Hex)  (?)   (PID)  (opener?)     (name) 
        0xFU    0      0       N/A       DeepStream

        In the above, our string would be 0xFU. Because that's what this really is,
        a stupid insult to all of this program's users.
        ''' 
        self.init_threads()
        self.window_id = int("03400001", 16)

        self.window = QWindow.fromWinId(self.window_id)
        self.ui.Deepstream_Window = QWidget.createWindowContainer(self.window, self)
        self.ui.Deepstream_Window.setObjectName("Deepstream_Window")
        self.ui.horizontalLayout.insertWidget(0, self.ui.Deepstream_Window)
        self.ui.Deepstream_Window.setMinimumSize(400, 400)
        self.ui.Deepstream_Window.setMaximumSize(16777215, 16777215)
        self.ui.Deepstream_Window.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.ui.time_updates.setReadOnly(True)
        
    def init_threads(self):
        self.master = master_thread()
        self.master.change_vel.connect(self.update_velocity)
        self.master.change_dist.connect(self.update_dist)
        self.master.change_status.connect(self.update_status)
        self.master.start()

    @pyqtSlot(float)
    def update_velocity(self, vel):
        self.ui.vel_value.setText("{0:.4f}".format(vel))

    @pyqtSlot(float)
    def update_dist(self, dist):
        self.ui.dist_value.setText("{0:.4f}".format(dist))

    @pyqtSlot('QString')
    def update_status(self, status):
        if status == "FIRING":
            self.ui.status_label.setStyleSheet('color: red')
        elif status == "TRACKING":
            self.ui.status_label.setStyleSheet("color: orange")
        else:
            self.ui.status_label.setStyleSheet("color: green")

        self.ui.status_label.setText(status)

    @pyqtSlot(int, int, int, int)
    def update_coords(x_pixel, y_pixel, x_angle, y_angle):
        insert_str = "{0}: X-Pixel: {1}, Y-Pixel: {2}, X-angle: {3}, Y-angle: {4}".format(time.time(), x_pixel, y_pixel, x_angle, y_angle)
        self.ui.time_updates.moveCursor(QTextCursor.Start)
        temp_str = self.ui.time_updates.insertPlainText(insert_str)
if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = GUI_TOP()
    win.show()
    sys.exit(app.exec_())
