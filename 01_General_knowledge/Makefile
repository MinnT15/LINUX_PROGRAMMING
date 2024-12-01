PRJ_NAME := hello
CUR_PATH := .
BIN_DIR := $(CUR_PATH)/bin
INC_DIR := $(CUR_PATH)/inc
SRC_DIR := $(CUR_PATH)/src
OBJ_DIR := $(CUR_PATH)/obj
LIB_DIR := $(CUR_PATH)/lib
LIB_SHARED_DIR := $(LIB_DIR)/shared
LIB_STATIC_DIR := $(LIB_DIR)/static

#!create object file

shared_objs:
	gcc -c -fPIC $(SRC_DIR)/hellovu.c    -o $(OBJ_DIR)/hellovu.o
	gcc -c -fPIC $(SRC_DIR)/helloworld.c    -o $(OBJ_DIR)/world.o
#!use -I to declare where the inc hello.h file is located
	gcc -c -fPIC $(CUR_PATH)/main.c    -o $(OBJ_DIR)/main.o -I $(INC_DIR) 

create_shared_lib:
#Name of lib always has "lib" front, and not same name of function
	gcc -shared $(OBJ_DIR)/hellovu.o $(OBJ_DIR)/world.o -o $(LIB_SHARED_DIR)/lib$(PRJ_NAME).so  

link_shared_lib: 
#!L: directory of lib
#!l name of lib
#!Link shared lib
	gcc $(OBJ_DIR)/main.o -L$(LIB_SHARED_DIR) -l$(PRJ_NAME) -o $(BIN_DIR)/use-shared-library


#! way 1: move lib to usr
install_shared_lib:
	install $(pwd)(LIB_SHARED_DIR)/lib$(PRJ_NAME).so /usr/lib 
#cp -f $(LIB_SHARED_DIR)/lib$(PRJ_NAME).so /usr/lib 
#! way 2: use environment variable LD_LIBRARY_PATH=$(pwd)/lib/shared ./bin/use-shared-library
all:shared_objs create_shared_lib link_shared_lib
	@echo "make all target"
clean:
	rm -rf $(OBJ_DIR)/*.o
	rm -rf $(LIB_SHARED_DIR)/lib$(PRJ_NAME).so
	rm -rf $(BIN_DIR)/use-shared-library
	












