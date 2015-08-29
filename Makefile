#
OBJ=client.o
TARGET=main
CC=gcc

$(TARGET):$(OBJ)
	$(CC) client.c net_utils.c mqtt.c packet.c -o test

clean:
	-rm -rf $(OBJ) $(TARGET) test
