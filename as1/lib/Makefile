CFLAGS := -g
targets = libecho.a
src = $(wildcard *.c)
objs = $(patsubst %.c,%.o,$(src))

all: $(targets)
.PHONY : all

libecho.a: $(objs)
	ar -c -r libecho.a $(objs)

$(objs) : $(src)

clean:
	rm $(objs) $(targets)
	