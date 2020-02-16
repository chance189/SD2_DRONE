# SD2_DRONE (Group 19)

Group 19's project involves using a Jetson Nano to track a drone, using a single camera sensor, with a known sized drone to calculate distance, velocity, and direction, utilized to shine a laser at the drone. The laser is proof of concept for firing upon the drone. The information is sent over serial to an Arduino Microcontroller, with a 8 bit CRC sent after every transmission to ensure proper data transactions.

## Getting Started

For this project, you require: 
* A Jetson Nano of course
* An Arduino MEGA2560, Arduino UNO, or a replication of our board
* A few cables and a logic converter (recommend any board with the TXS0108E, it's great)

## Prerequisites

This project requires ZeroMQ, Protobuffers for the Jetson, and ProtoThreading Libraries for the Arduino

The project requires ZeroMQ for C and for python, use libzmq: https://zeromq.org/languages/
For protobuffers, the python install can be followed here: https://developers.google.com/protocol-buffers
For protobuffers using C, use the github install directions: https://github.com/protobuf-c/protobuf-c

## Installing and Running Software

move to make entire project, run the following

```
cd Protobuffers
make
cd ../
cd deepstream_m_proof_using_config
make -C nvdsinver_custom_impl_Yolo
make
cd ..
./runProg.sh
```

## Description/How it works

I'll think of something later

## Authors
Chance Reimer
Joseph Musante
Calvin Sands

## Acknowledgments

Thank you google, stack exchange, and all the people who asked relevant questions on forums. I love you all.



