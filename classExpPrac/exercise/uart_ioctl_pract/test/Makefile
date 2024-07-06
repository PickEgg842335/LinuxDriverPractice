TARGET=test
TARGET1=test_read

all: $(TARGET)

CC = gcc

CFLAGS = -g -O2 -Wall

all: $(TARGET) $(TARGET1)

$(TARGET): $(TARGET).c
	$(CC) $(TARGET).c  -o $(TARGET)

$(TARGET1): $(TARGET1).c
	$(CC) $(TARGET1).c  -o $(TARGET1)

clean:
	rm -f $(TARGET) $(TARGET1)
