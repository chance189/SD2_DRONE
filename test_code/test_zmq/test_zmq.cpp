#include <zmq.hpp>
#include <iostream>
#include <unistd.h>
#include "detect_pipe.pb.h"
#include <fstream>

//using namespace PROTOBUF_NAMESPACE_ID;
int main() {
    //  Prepare our context and socket
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_PUSH);
    socket.bind("tcp://*:5555");
    SD1::DETECTION* d = new SD1::DETECTION();           //Initialize our detector
    // forever loop
    while (true) {
        zmq::message_t request;

        //  Wait for next request from client
        //socket.recv(&request);
        //std::string replyMessage = std::string(static_cast<char *>(request.data()), request.size());
//        std::string replyMessage = std::string((request.data())., request.size());
        // Print out received message
	//d->ParseFromString(replyMessage);
        //std::cout << "SENT DATA || x: " << d->x() << "y: " << d->y() << "h: " << d->h() << "w: " << d->w() << "label: " + d->label() << std::endl;
	d->set_x(44);
	d->set_y(44);
        d->set_h(55);
        d->set_w(66);
        d->set_label("PLEASE KILL ME");


        //  Send reply back to client
        std::string msgToClient;
	d->SerializeToString(&msgToClient);
        char arr[msgToClient.size()];
        memcpy((void *) arr, (msgToClient.c_str()), msgToClient.size());
        //zmq::message_t reply(msgToClient.size());
        //memcpy((void *) reply.data(), (msgToClient.c_str()), msgToClient.size());
        //socket.send(reply, 1);
        //zmq_send(&socket, &reply, ZMQ_NOBLOCK, 0);
        int send = zmq_send(socket, arr, msgToClient.size(), ZMQ_NOBLOCK);
        std::cout << "WE SENT: " << send << std::endl;
    }
    return 0;
}
