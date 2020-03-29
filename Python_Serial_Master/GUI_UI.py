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
        self.status_label.setMinimumSize(QtCore.QSize(375, 0))
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
        self.dist_label = QtWidgets.QLabel(self.centralwidget)
        self.dist_label.setMinimumSize(QtCore.QSize(150, 0))
        self.dist_label.setMaximumSize(QtCore.QSize(150, 50))
        self.dist_label.setObjectName("dist_label")
        self.gridLayout_2.addWidget(self.dist_label, 3, 0, 1, 1)
        self.drone_id_val = QtWidgets.QLabel(self.centralwidget)
        self.drone_id_val.setMinimumSize(QtCore.QSize(150, 0))
        self.drone_id_val.setMaximumSize(QtCore.QSize(150, 20))
        font = QtGui.QFont()
        font.setFamily("Franklin Gothic Heavy")
        font.setPointSize(10)
        self.drone_id_val.setFont(font)
        self.drone_id_val.setAlignment(QtCore.Qt.AlignCenter)
        self.drone_id_val.setObjectName("drone_id_val")
        self.gridLayout_2.addWidget(self.drone_id_val, 1, 1, 1, 1)
        self.drone_id_label = QtWidgets.QLabel(self.centralwidget)
        self.drone_id_label.setMinimumSize(QtCore.QSize(150, 0))
        self.drone_id_label.setMaximumSize(QtCore.QSize(150, 20))
        font = QtGui.QFont()
        font.setFamily("Franklin Gothic Heavy")
        font.setPointSize(10)
        self.drone_id_label.setFont(font)
        self.drone_id_label.setAlignment(QtCore.Qt.AlignCenter)
        self.drone_id_label.setObjectName("drone_id_label")
        self.gridLayout_2.addWidget(self.drone_id_label, 1, 0, 1, 1)
        self.dist_value = QtWidgets.QLabel(self.centralwidget)
        self.dist_value.setMinimumSize(QtCore.QSize(150, 0))
        self.dist_value.setMaximumSize(QtCore.QSize(150, 50))
        self.dist_value.setAlignment(QtCore.Qt.AlignCenter)
        self.dist_value.setObjectName("dist_value")
        self.gridLayout_2.addWidget(self.dist_value, 3, 1, 1, 1)
        self.vel_value = QtWidgets.QLabel(self.centralwidget)
        self.vel_value.setMinimumSize(QtCore.QSize(150, 0))
        self.vel_value.setMaximumSize(QtCore.QSize(150, 50))
        self.vel_value.setAlignment(QtCore.Qt.AlignCenter)
        self.vel_value.setObjectName("vel_value")
        self.gridLayout_2.addWidget(self.vel_value, 2, 1, 1, 1)
        self.vel_label = QtWidgets.QLabel(self.centralwidget)
        self.vel_label.setMinimumSize(QtCore.QSize(150, 0))
        self.vel_label.setMaximumSize(QtCore.QSize(150, 50))
        self.vel_label.setObjectName("vel_label")
        self.gridLayout_2.addWidget(self.vel_label, 2, 0, 1, 1)
        self.verticalLayout.addLayout(self.gridLayout_2)
        self.text_edit_label = QtWidgets.QLabel(self.centralwidget)
        self.text_edit_label.setMaximumSize(QtCore.QSize(400, 20))
        self.text_edit_label.setAlignment(QtCore.Qt.AlignCenter)
        self.text_edit_label.setObjectName("text_edit_label")
        self.verticalLayout.addWidget(self.text_edit_label)
        self.time_updates = QtWidgets.QPlainTextEdit(self.centralwidget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.time_updates.sizePolicy().hasHeightForWidth())
        self.time_updates.setSizePolicy(sizePolicy)
        self.time_updates.setMaximumSize(QtCore.QSize(400, 16777215))
        self.time_updates.setStyleSheet("color: rgb(188, 0, 0)")
        self.time_updates.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.time_updates.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.time_updates.setObjectName("time_updates")
        self.verticalLayout.addWidget(self.time_updates)
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
        self.actionHide_Status.setCheckable(True)
        self.actionHide_Status.setObjectName("actionHide_Status")
        self.menu_adv.addAction(self.actionHide_Status)
        self.menubar.addAction(self.menu_adv.menuAction())

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "MainWindow"))
        self.status_label.setText(_translate("MainWindow", "<html><head/><body><p><span style=\" font-size:20pt; color:#00ff00;\">IDLE</span></p></body></html>"))
        self.dist_label.setText(_translate("MainWindow", "<html><head/><body><p><span style=\" color:#ffffff;\">Distance (inches)</span></p></body></html>"))
        self.drone_id_val.setText(_translate("MainWindow", "N/A"))
        self.drone_id_label.setText(_translate("MainWindow", "Drone ID:"))
        self.dist_value.setText(_translate("MainWindow", "0.00"))
        self.vel_value.setText(_translate("MainWindow", "0.00"))
        self.vel_label.setText(_translate("MainWindow", "<html><head/><body><p><span style=\" color:#ffffff;\">Velocity (inches/sec)</span></p></body></html>"))
        self.text_edit_label.setText(_translate("MainWindow", "Timestamp Updates"))
        self.time_updates.setPlainText(_translate("MainWindow", "Status Updates"))
        self.menu_adv.setTitle(_translate("MainWindow", "Advanced"))
        self.actionHide_Status.setText(_translate("MainWindow", "Hide Status"))
        self.actionHide_Status.setShortcut(_translate("MainWindow", "Ctrl+S"))

