program_name := sticksc

CC := gcc
CC_FLAGS := -ggdb -Wall -Werror
LD_FLAGS := -pthread

SRCS := main.c game.c server.c
OBJS := $(SRCS:%.c=%.o)
HDRS := game.h server.h error.h

%.o: %.c
	${CC} -c $(CC_FLAGS) $<

all: $(program_name)

$(program_name): $(OBJS) $(HDRS)
	$(CC) $(LD_FLAGS) -o $@ $(OBJS)

clean:
	rm -f *.o $(program_name)
