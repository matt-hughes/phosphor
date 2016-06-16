SRC = main.cpp

#CC ?= g++

INCLUDE = -I/usr/local/include
LIBS = -L/usr/local/lib
CFLAGS = -w -g
LDFLAGS = -framework OpenGL -lglfw3 -lglew

APP_NAME = phosphor

BUILD_DIR = ./build

APP_OUT = $(BUILD_DIR)/$(APP_NAME)

#This is the target that compiles our executable
all: $(APP_OUT)

$(APP_OUT): $(SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) $^ $(INCLUDE) $(LIBS) $(CFLAGS) $(LDFLAGS) -o $@

clean:
	rm $(APP_OUT)