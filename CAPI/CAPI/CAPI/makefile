CPPCOMPILER = g++ -I./include/linux -std=c++17 -O2 -Wall

SOURCE_DIR?=./src
BIN_DIR?=.
PROTO_DIR = ./src/proto

# remove the last / if it exits
SOURCE_DIR_NO_SUFFIX=$(shell echo $(SOURCE_DIR) | sed "s/\/$$//g")
BIN_DIR_NO_SUFFIX=$(shell echo $(BIN_DIR) | sed "s/\/$$//g")
BIN_DIR_IN_SED=$(shell echo $(BIN_DIR_NO_SUFFIX)| sed 's/[./]/\\&/g')
SOURCE_DIR_IN_SED=$(shell echo $(SOURCE_DIR_NO_SUFFIX)| sed 's/[./]/\\&/g')

# no prefix
PLAYER_SOURCES= $(shell ls $(SOURCE_DIR_NO_SUFFIX) | grep -E 'player[1-4]?.cpp' |sed ":a;N;s/\n/ /g;ba")
PLAYER_OBJECTS=$(shell echo $(PLAYER_SOURCES)| sed "s/cpp/o/g")

CPP_OBJECTS = CAPI.o API.o Logic.o main.o
PROTO_OBJECTS = Message2Client.pb.o Message2Server.pb.o MessageType.pb.o 
objects = $(CPP_OBJECTS)
objects += $(PROTO_OBJECTS)

CPP_DEPENDENCIES = $(patsubst %.o,%.d,$(CPP_OBJECTS))

AI_O_PREREQUISITES= ./src/Base.h ./src/Constants.h ./src/Base.h

all: $(objects) player_obj
	for i in $(PLAYER_OBJECTS);\
	do\
		g++ -o  $(BIN_DIR_NO_SUFFIX)/$$(echo $$i| sed "s/.o//g"| sed "s/[a-z]/\u&/g") \
		$$(echo $(objects) | sed -E "s/^| /&$(BIN_DIR_IN_SED)\//g") $(BIN_DIR_NO_SUFFIX)/$$i\
		-L./a -lprotobuf -lhpsocket \
		-Wl,-rpath=./so -std=c++17 -pthread -O2 -Wall;\
	done

$(CPP_OBJECTS): %.o: $(SOURCE_DIR)/%.cpp
	$(CPPCOMPILER) -o $(BIN_DIR)/$@ -c $<

$(PROTO_OBJECTS): %.o: $(PROTO_DIR)/%.cc
	$(CPPCOMPILER) -o $(BIN_DIR)/$@ -c $<

$(CPP_DEPENDENCIES): %.d: $(SOURCE_DIR)/%.cpp
	$(CPPCOMPILER) -o $@ -MM $<

include $(CPP_DEPENDENCIES)

.PHONY:clean player_obj

clean:
	-rm $$(echo $(objects) | sed -E "s/^| /&$(BIN_DIR_IN_SED)\//g")\
	 $$(echo $(PLAYER_OBJECTS) | sed -E "s/^| /&$(BIN_DIR_IN_SED)\//g") \
	 $(shell echo $(PLAYER_OBJECTS)| sed "s/.o//g"|sed -E "s/^| /&$(BIN_DIR_IN_SED)\//g"| sed "s/[a-z]/\u&/g")
	-rm $(CPP_DEPENDENCIES)

player_obj: $(shell echo $(PLAYER_SOURCES)|sed -E "s/^| /&$(SOURCE_DIR_IN_SED)\//g")
	for i in $(PLAYER_SOURCES);\
	do\
		g++ -c $(SOURCE_DIR_NO_SUFFIX)/$$i -o $(BIN_DIR_NO_SUFFIX)/$$(echo $$i|sed "s/cpp/o/g")\
		 -I./src/ -std=c++17 -O2 -Wall;\
	done
