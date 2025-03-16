program_name := sticksc

CC := gcc
C_FLAGS := -ggdb -Wall -Werror

SRCS := main.c game.c server.c
OBJS := $(SRCS:%.c=%.o)
HDRS := game.h server.c

%.o: %.c
	${CC} -c $(CC_FLAGS) $<

all: $(program_name)

$(program_name): $(OBJS) $(HDRS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f *.o $(program_name)
