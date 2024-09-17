CC = gcc

CFLAGS = -Wall -g

LDFLAGS = -lglut -lGL -lGLU -lm

# $(wildcard *.c)
SRCS = main.c

TARGET = guache

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
	rm -f *.o

run: $(TARGET)
	./$(TARGET)

