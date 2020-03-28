import zmq
import detect_pipe_pb2
import time
import sys
import os

context = zmq.Context()
socket = context.socket(zmq.PULL)
port = "5555"
socket.connect ("tcp://localhost:%s" % port)

d = detect_pipe_pb2.DETECTION()
d.x = 40
d.y = 50
d.w = 55
d.h = 66
d.label = "CONNECTION"
poller = zmq.Poller()
poller.register(socket)
while True:
#for i in range(20):
    #socket.send(d.SerializeToString())
    socks = dict(poller.poll(0))
    if socket in socks:   
    #try:
        message = socket.recv()
        d.ParseFromString(message)
        print ("Received reply from server: x: {0}, y: {1}, w: {2}, h: {3}, label: {4}".format(d.x, d.y, d.h, d.w, d.label))
    #except zmq.Again:
        #time.sleep(1)
    else:
        #socket.send(d.SerializeToString())
        print("I'm so lonely")
        time.sleep(0.15)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("INTERRUPTED")
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)
