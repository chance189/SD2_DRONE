#Chance's copied makefile

APP:= ckr-test-c-protobuf


SRCS:= $(wildcard *.c)
INCS:= $(wildcard *.h)

CFLAGS+= -pthread -I/usr/local/include
LIBS+=   -L/usr/local/lib -lprotobuf -lzmq -lprotobuf-c

PKGS:= glib-2.0

CFLAGS+= `pkg-config --cflags $(PKGS)`
LIBS+= `pkg-config --libs $(PKGS)`

OBJS:= $(SRCS:.c=.o)

all: $(APP)
%.o: %.c $(INCS) Makefile
	$(CC) -c -o $@ $(CFLAGS) $<

$(APP): $(OBJS) Makefile
	$(CC) -o $(APP) $(OBJS) $(LIBS)

clean:
	rm -rf $(OBJS) $(APP)
