import time
import serial


ser = serial.Serial()
ser.port = "/dev/ttyTHS1"
ser.baudrate = 9600
ser.timeout = 1
ser.setDTR(False)
ser.setRTS(False)
ser.open()

while True:
    ser.write("f".encode("ascii"))
    val = ser.read(1)
    print("Sent: {0}, Rx: {1}".format("f".encode("ascii"), val))
    time.sleep(2)
