CFLAGS := -g
targets=server
src = server.c
objs = $(patsubst %.c,%.o,$(src))
libs = -L../lib -lecho -lpthread

all: $(targets)
.PHONY: all

server: $(objs) ../lib/libecho.a
	gcc $(CFLAGS) -o server $(objs) $(libs)

$(objs) : $(src)

clean:
	rm $(objs) $(targets)
