# mqttc-bridge Makefile
# uname.github.com/mqtt-bridge

OBJ:=$(patsubst %.c,%.o,$(wildcard *.c))
TARGET=mqtt-bridge
CC=mips-openwrt-linux-gcc

$(TARGET): $(OBJ)
	$(CC) -o $@ $(OBJ)

clean:
	-rm -rf $(OBJ) $(TARGET)
