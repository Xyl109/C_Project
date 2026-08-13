CC      = gcc
CFLAGS  = -Wall -Wextra
TARGET  = output/libman.exe
SRCS    = main.c book.c borrow.c query.c admin.c user.c blacklist.c
OBJS    = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean run
clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)
