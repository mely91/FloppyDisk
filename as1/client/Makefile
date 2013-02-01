CFLAGS := -g
targets=client
src = client.c
objs = $(patsubst %.c,%.o,$(src))
libs = -L../lib -lecho

all: $(targets)
.PHONY: all

client: $(objs) ../lib/libecho.a
	gcc $(CFLAGS) -o client $(objs) $(libs)

$(objs) : $(src)

clean:
	rm $(objs) $(targets)
