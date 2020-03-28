# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'mainwindow.ui'
#
# Created by: PyQt5 UI code generator 5.10.1
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(800, 600)
        MainWindow.setStyleSheet("background-color: rgb(0, 0, 0);\n"
"color: rgb(255, 255, 255);\n"
"\n"
"QMenuBar {\n"
"    backgroud-color: rgb(2, 45, 115);\n"
"    color: rgb(255, 255, 255);\n"
"    border: 1px solid #000;\n"
"}\n"
"QMenuBar::item {\n"
"    background-color: rgb(49,49,49);\n"
"    color: rgb(255,255,255);\n"
" }\n"
"\n"
" QMenuBar::item::selected {\n"
"     background-color: rgb(30,30,30);\n"
" }\n"
"\n"
"QMenu {\n"
"   background-color: rgb(49,49,49);\n"
"   color: rgb(255,255,255);\n"
"   border: 1px solid #000;           \n"
" }\n"
"\n"
"QMenu::item::selected {\n"
"   background-color: rgb(30,30,30);\n"
"}")
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.gridLayout = QtWidgets.QGridLayout(self.centralwidget)
        self.gridLayout.setObjectName("gridLayout")
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.Deepstream_Window = QtWidgets.QWidget(self.centralwidget)
        self.Deepstream_Window.setObjectName("Deepstream_Window")
        self.horizontalLayout.addWidget(self.Deepstream_Window)
        self.verticalLayout = QtWidgets.QVBoxLayout()
        self.verticalLayout.setObjectName("verticalLayout")
        self.status_label = QtWidgets.QLabel(self.centralwidget)
        self.status_label.setMaximumSize(QtCore.QSize(400, 100))
        font = QtGui.QFont()
        font.setFamily("Arial Rounded MT Bold")
        font.setPointSize(16)
        self.status_label.setFont(font)
        self.status_label.setAlignment(QtCore.Qt.AlignCenter)
        self.status_label.setObjectName("status_label")
        self.verticalLayout.addWidget(self.status_label)
        self.gridLayout_2 = QtWidgets.QGridLayout()
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.vel_label = QtWidgets.QLabel(self.centralwidget)
        self.vel_label.setMaximumSize(QtCore.QSize(150, 50))
        self.vel_label.setObjectName("vel_label")
        self.gridLayout_2.addWidget(self.vel_label, 0, 0, 1, 1)
        self.dist_label = QtWidgets.QLabel(self.centralwidget)
        self.dist_label.setMaximumSize(QtCore.QSize(100, 50))
        self.dist_label.setObjectName("dist_label")
        self.gridLayout_2.addWidget(self.dist_label, 1, 0, 1, 1)
        spacerItem = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.gridLayout_2.addItem(spacerItem, 2, 0, 1, 1)
        spacerItem1 = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.gridLayout_2.addItem(spacerItem1, 2, 1, 1, 1)
        self.vel_value = QtWidgets.QLabel(self.centralwidget)
        self.vel_value.setMaximumSize(QtCore.QSize(150, 50))
        self.vel_value.setAlignment(QtCore.Qt.AlignCenter)
        self.vel_value.setObjectName("vel_value")
        self.gridLayout_2.addWidget(self.vel_value, 0, 1, 1, 1)
        self.dist_value = QtWidgets.QLabel(self.centralwidget)
        self.dist_value.setMaximumSize(QtCore.QSize(150, 50))
        self.dist_value.setAlignment(QtCore.Qt.AlignCenter)
        self.dist_value.setObjectName("dist_value")
        self.gridLayout_2.addWidget(self.dist_value, 1, 1, 1, 1)
        self.verticalLayout.addLayout(self.gridLayout_2)
        self.horizontalLayout.addLayout(self.verticalLayout)
        self.gridLayout.addLayout(self.horizontalLayout, 0, 0, 1, 1)
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 800, 26))
        self.menubar.setObjectName("menubar")
        self.menu_adv = QtWidgets.QMenu(self.menubar)
        self.menu_adv.setObjectName("menu_adv")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)
        self.actionHide_Status = QtWidgets.QAction(MainWindow)
        self.actionHide_Status.setObjectName("actionHide_Status")
        self.menu_adv.addAction(self.actionHide_Status)
        self.menubar.addAction(self.menu_adv.menuAction())

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "MainWindow"))
        self.status_label.setText(_translate("MainWindow", "<html><head/><body><p><span style=\" color:#00ff00;\">IDLE</span></p></body></html>"))
        self.vel_label.setText(_translate("MainWindow", "<html><head/><body><p><span style=\" color:#ffffff;\">Velocity (inches/sec)</span></p></body></html>"))
        self.dist_label.setText(_translate("MainWindow", "<html><head/><body><p><span style=\" color:#ffffff;\">Distance (inches)</span></p></body></html>"))
        self.vel_value.setText(_translate("MainWindow", "0.00"))
        self.dist_value.setText(_translate("MainWindow", "0.00"))
        self.menu_adv.setTitle(_translate("MainWindow", "Advanced"))
        self.actionHide_Status.setText(_translate("MainWindow", "Hide Status"))

