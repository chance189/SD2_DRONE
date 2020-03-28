SRCDIR = /home/chance/Desktop/SD2_DRONE/Protobuffers
SVDIR = ../

create_proto:
	protoc -I=$(SRCDIR) --cpp_out=$(SVDIR) --python_out=$(SVDIR) --c_out=$(SVDIR) $(SRCDIR)/detect_pipe.proto
