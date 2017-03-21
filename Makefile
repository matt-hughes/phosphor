#TARGET_RPI?=1

ifeq ($(shell uname -m),armv7l)
TARGET_RPI=1
endif

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

#RUN_CMD = $(APP_OUT) --font1 PetASCII4_mono_glow.tga --fontLayout1 0,0,39,47,39,47,11,11,27,35 --fontColor 60,195,60 --res 1920,1080
RUN_CMD = $(APP_OUT) --font2 PetASCII4_mono_glow.tga --fontLayout2 0,0,39,47,39,47,11,11,27,35 --fontColor 60,195,60 --res 1920,1080

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

runusb: $(APP_OUT)
	$(RUN_CMD)

rundemo: $(APP_OUT)
	$(RUN_CMD) --demo

run:
	$(RUN_CMD)
