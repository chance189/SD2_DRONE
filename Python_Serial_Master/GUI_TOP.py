'''
# Author: Chance Reimer
# Purpose: To create a GUI that will show status of the drone
# Note: I'm so sorry.
'''
from GUI_UI import *  #because why select what you need? We are american after all, TAKE IT ALL
from PyQt5.QtWidgets import QWidget, QLabel, QApplication, QMainWindow, QVBoxLayout, QSizePolicy, QPlainTextEdit, QToolTip
from PyQt5.QtCore import QThread, Qt, pyqtSignal, pyqtSlot, QRect
from PyQt5.QtGui import QImage, QPixmap, QWindow, QTextCursor
from master_thread import master_thread
import sys
import time
import math

class GUI_TOP(QMainWindow):

    def __init__(self):
        super().__init__()
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        #Tie action socket for toggling menu
        self.ui.actionHide_Status.triggered.connect(self.handle_hide_menu)
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
        self.window_id = int("03200001", 16)
        
        '''
        self.window = QWindow.fromWinId(self.window_id)
        self.ui.Deepstream_Window = QWidget.createWindowContainer(self.window, self)
        self.ui.Deepstream_Window.setObjectName("Deepstream_Window")
        self.ui.horizontalLayout.insertWidget(0, self.ui.Deepstream_Window)
        self.ui.Deepstream_Window.setMinimumSize(400, 400)
        self.ui.Deepstream_Window.setMaximumSize(16777215, 16777215)
        self.ui.Deepstream_Window.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        '''
        self.ui.time_updates.setReadOnly(True)
        self.ui.time_updates.setMaximumBlockCount(50)
        
    def init_threads(self):
        #init
        self.master = master_thread()
        #tie slots to signals
        self.master.change_vel.connect(self.update_velocity)
        self.master.change_dist.connect(self.update_dist)
        self.master.change_status.connect(self.update_status)
        self.master.update_reporting.connect(self.update_coords)
        self.master.change_drone_id.connect(self.update_drone_id)
        self.master.lost_drone.connect(self.handle_timeout_detection)
        #start
        self.master.start()

    #Simple rounding, shift number to int and round using ceil, shift back
    def round(self, in_val , shift_val=0):
        shift_num = 10**shift_val
        return math.ceil(in_val*shift_num)/shift_num   #shift left (multiply) then shift back
    
    #bool to toggle visibility of GUI menu
    def set_HUD_visible(self, tf):
            self.ui.status_label.setVisible(tf)
            self.ui.dist_label.setVisible(tf)
            self.ui.dist_value.setVisible(tf)
            self.ui.vel_label.setVisible(tf)
            self.ui.vel_value.setVisible(tf)
            self.ui.drone_id_label.setVisible(tf)
            self.ui.drone_id_val.setVisible(tf)
            self.ui.time_updates.setVisible(tf)
            self.ui.text_edit_label.setVisible(tf)

    '''
    * Slots below this line
    '''
    @pyqtSlot(int)
    def handle_timeout_detection(self, drone_id):
        if drone_id == self.drone_id:
            self.update_status("IDLE")  #update the status
            self.ui.vel_value.setText("{:0>8}".format(0))
            self.ui.dist_value.setText("{:0>8}".format(0))
            self.ui.drone_id_val.setText("N/A")
            self.ui.time_updates.document().setPlainText("")

        #move to bottom center of screen
        #rect_screen = QApplication.desktop().availableGeometry(self)
        rect_screen = self.geometry()
        center = rect_screen.bottomLeft()
        center.setX(center.x()+rect_screen.width()*0.5)
        QToolTip.showText(center, "LOST TRACK OF DRONE{0}!".format(drone_id), self)

    @pyqtSlot(bool)
    def handle_hide_menu(self, checked):
        self.set_HUD_visible(not checked)
            

    @pyqtSlot(float)
    def update_velocity(self, vel):
        self.ui.vel_value.setText("{:0>8}".format(int(vel)))

    @pyqtSlot(float)
    def update_dist(self, dist):
        self.ui.dist_value.setText("{:0>8}".format(self.round(dist, 0)))

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
    def update_coords(self, x_pixel, y_pixel, x_angle, y_angle):
        insert_str = "{0}: X-pix: {1:04d}, Y-pix: {2:04d}, X-deg: {3:03d}, Y-deg: {4:03d}\n".format(time.strftime("%T", time.localtime()), x_pixel, y_pixel, x_angle, y_angle)
        temp_str = self.ui.time_updates.appendPlainText(insert_str)
        self.ui.time_updates.moveCursor(QTextCursor.End)

    @pyqtSlot(int)
    def update_drone_id(self, drone_id):
        self.drone_id = drone_id
        self.ui.drone_id_val.setText("{0}".format(drone_id))

if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = GUI_TOP()
    win.show()
    sys.exit(app.exec_())
