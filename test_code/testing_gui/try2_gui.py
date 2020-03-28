'''
# Alright, last one was bad, let's try to place the screen in a pyqt window
'''
from PyQt5.QtWidgets import QWidget, QLabel, QApplication, QMainWindow, QVBoxLayout
from PyQt5.QtCore import QThread, Qt, pyqtSignal, pyqtSlot
from PyQt5.QtGui import QImage, QPixmap, QWindow

import subprocess 
import time
import os
import sys

class trap_deepstream(QMainWindow):
    def __init__(self, parent=None):
        QMainWindow.__init__(self, parent=parent)
        self.initUI()

    def initUI(self):
        #os.chdir("../deepstream_proof_using_config")
        #print("Changed dir to: {0}".format(os.getcwd()))
        #deepstream = subprocess.call(["./ckr-deepstream-app", "-c", "deepstream_app_config_yoloV3_tiny.txt"])
        windowid = int("03e00001", 16)
        #time.sleep(30)
        self.window = QWindow.fromWinId(windowid)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.widget = QWidget.createWindowContainer(self.window, self)
        self.widget.resize(1280, 720)
        print(self.window.parent())
        layout = QVBoxLayout()
        layout.addWidget(self.widget)
        self.setGeometry(100, 100, 900, 900)
        self.setWindowTitle('UI')
        self.show()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = trap_deepstream()
    sys.exit(app.exec_())
