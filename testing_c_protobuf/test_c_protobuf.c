#include <zmq.h>
#include "detect_pipe.pb-c.h"
#include <stdlib.h>
#include <glib.h>

void* socket;


int main(int argc, char*argv[])
{
	void * context = zmq_ctx_new();
	socket = zmq_socket(context, ZMQ_PUSH);
	int rc = zmq_bind(socket, "tcp://*:5555");
	assert(rc==0);

	g_print("So we should have 0: %d\n", rc);

	SD1__DETECTION message = SD1__DETECTION__INIT;
	g_print("INIT GOOD?\n");
	g_print("we got the packed size??\n");
	char name [5] = "TEST";
	message.x = 100;
	message.y = 200;
	message.h = 10;
	message.w = 20;
	message.label = name;
	g_print("initialized message");
	size_t size = sd1__detection__get_packed_size(&message);
	char* out;
	out = malloc(size);
	sd1__detection__pack(&message, out);
	int i = 0;
	while(i < 1000){
		zmq_send(socket, (char*)out, size, 0);
		i++;
	}

	free(out);
	g_print("Finished");
	return 0;
}
