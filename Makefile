#TARGET_RPI?=1

SRC = \
	main.cpp \
	fake6502.c \
	usbSerialDecoder.c \
	frameReadThread.c \
	emu.c

#CC ?= g++

INCLUDE = -I/usr/local/include
DEFINE = -DUSE_USB_SERIAL
LIBS = -L/usr/local/lib
CFLAGS = -w -g -O3
LDFLAGS =

APP_NAME = phosphor

BUILD_DIR = ./build

APP_OUT = $(BUILD_DIR)/$(APP_NAME)

ifeq ($(TARGET_RPI),1)
SRC += rpi_egl.c
INCLUDE += -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux/ -I/opt/vc/include/
LDFLAGS += -L/opt/vc/lib/ -lbcm_host -lEGL -lGLESv2 -lm -lpthread
DEFINE += -DTARGET_RPI
else
LDFLAGS += -framework OpenGL -lglfw3 -lglew
DEFINE += -DTARGET_OSX
endif

#This is the target that compiles our executable
all: $(APP_OUT)

$(APP_OUT): $(SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) $^ $(INCLUDE) $(DEFINE) $(LIBS) $(CFLAGS) $(LDFLAGS) -o $@

clean:
	rm $(APP_OUT)