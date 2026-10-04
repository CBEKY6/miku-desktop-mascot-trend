CC = gcc
CFLAGS = -Wall -std=c99
LIBS = -lraylib -lX11 -lGL -lm -lpthread -ldl -lrt

TARGET = miku
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
 $(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
 rm -f $(TARGET)

run: all
 ./$(TARGET)
