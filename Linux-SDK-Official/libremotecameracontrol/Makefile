
ROOT_DIR ?= .
OUT ?= ../out
OUT_DIR := ${OUT}/lib

CC := gcc
SYS := $(shell $(CC) -dumpmachine)

ifneq (, $(findstring linux, $(SYS)))
# Linux
	tg_so = ${OUT_DIR}/libremotecameracontrol.so
else ifneq (, $(findstring cygwin, $(SYS)))
# Cygwin
	tg_so = ${OUT_DIR}/libremotecameracontrol.dll
else
#	exit 1
endif

override INCLUDES += -I${ROOT_DIR}/include
override INCLUDES += -I${ROOT_DIR}/ports

sources_usb:= ${ROOT_DIR}/ports/ports_usb_impl.cpp
lib_usb:= -lusb-1.0

sources_so :=
sources_so += ${sources_usb}
sources_so += ${ROOT_DIR}/ports/ports_ptp_impl.cpp
sources_so += ${ROOT_DIR}/sources/socc_ptp.cpp
sources_so += ${ROOT_DIR}/sources/parser.cpp
OBJ_DIR := .obj
OBJECTS := $(addprefix $(OBJ_DIR)/, $(notdir $(sources_so:.cpp=.o)))

.PHONY: clean

all: ${OUT_DIR} $(OBJ_DIR) ${tg_so}

objs: $(OBJ_DIR) $(OBJECTS)

${OUT_DIR}:
	mkdir -p ${OUT_DIR}

${tg_so}: $(OBJECTS)
	${CC} $^ --share -o $@ ${lib_usb} -pthread -lstdc++

$(OBJ_DIR)/%.o : **/%.cpp
	$(CC) $(CFLAGS) -fPIC -g $(INCLUDES) -o $@ -c $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf ${tg_so} $(OBJ_DIR)
