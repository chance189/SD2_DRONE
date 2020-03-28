'''
# Author: Chance Reimer
# Purpose: To create a GUI that will show status of the drone
# Note: I'm so sorry.
'''
from GUI_UI import *  #because why select what you need? We are american after all, TAKE IT ALL
from PyQt5.QtWidgets import QWidget, QLabel, QApplication, QMainWindow, QVBoxLayout
from PyQt5.QtCore import QThread, Qt, pyqtSignal, pyqtSlot
from PyQt5.QtGui import QImage, QPixmap, QWindow

class GUI_TOP(QMainWindow):
    def __init__(self):
        super().__init__()
        self.ui = Ui_Main_Window()
        self.setupUi(self)

        #Tie some some outputs to these things

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
        self.window_id = int("deadbeaf", 16)

        self.window = QWindow.fromWinId(window_id)
        self.ui.Deepstream_Window = QWidget.createWindowContainer(self.window, self)
        self.widget.resize(1280, 720)
        
    def init_threads(self):
        self.QThread


if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = GUI_TOP()
    sys.exit(ap.exec_())
