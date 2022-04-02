CPPCOMPILER = g++ -I./include/linux -std=c++17 -O2 -Wall

SOURCE_DIR?=./src
BIN_DIR?=.
API_DIR?=./src
PROTO_DIR = $(API_DIR)/proto

# remove the last / if it exits
SOURCE_DIR_NO_SUFFIX=$(shell echo $(SOURCE_DIR) | sed "s/\/$$//g")
BIN_DIR_NO_SUFFIX=$(shell echo $(BIN_DIR) | sed "s/\/$$//g")
BIN_DIR_IN_SED=$(shell echo $(BIN_DIR_NO_SUFFIX)| sed 's/[./]/\\&/g')
SOURCE_DIR_IN_SED=$(shell echo $(SOURCE_DIR_NO_SUFFIX)| sed 's/[./]/\\&/g')

# no prefix
PLAYER_SOURCES= $(shell ls $(SOURCE_DIR_NO_SUFFIX) | grep -E 'player[1-4]?.cpp' |sed ":a;N;s/\n/ /g;ba")
PLAYER_OBJECTS=$(shell echo $(PLAYER_SOURCES)| sed "s/cpp/o/g")

CPP_OBJECTS = CAPI.o API.o Logic.o main.o
CPP_OBJECTS_ALL_PATH = $(patsubst %.o,$(BIN_DIR)/%.o,$(CPP_OBJECTS))
PROTO_OBJECTS = Message2Client.pb.o Message2Server.pb.o MessageType.pb.o 
PROTO_OBJECTS_ALL_PATH = $(patsubst %.o,$(BIN_DIR)/%.o,$(PROTO_OBJECTS))
objects = $(CPP_OBJECTS)
objects += $(PROTO_OBJECTS)
objects_all_path = $(CPP_OBJECTS_ALL_PATH)
objects_all_path += $(PROTO_OBJECTS_ALL_PATH)

CPP_DEPENDENCIES = $(patsubst %.o,$(BIN_DIR)/%.d,$(CPP_OBJECTS))

AI_O_PREREQUISITES= $(SOURCE_DIR)/Base.h $(SOURCE_DIR)/Constants.h $(SOURCE_DIR)/Base.h

all: $(objects_all_path) player_obj
	for i in $(PLAYER_OBJECTS);\
	do\
		g++ -o  $(BIN_DIR_NO_SUFFIX)/$$(echo $$i| sed "s/.o//g"| sed "s/[a-z]/\u&/g") \
		$$(echo $(objects) | sed -E "s/^| /&$(BIN_DIR_IN_SED)\//g") $(BIN_DIR_NO_SUFFIX)/$$i\
		-L./a -lprotobuf -lhpsocket \
		-Wl,-rpath=./so -std=c++17 -pthread -O2 -Wall;\
	done

$(CPP_OBJECTS_ALL_PATH): $(BIN_DIR)/%.o: $(API_DIR)/%.cpp
	$(CPPCOMPILER) -o $@ -c $<

$(PROTO_OBJECTS_ALL_PATH): $(BIN_DIR)/%.o: $(PROTO_DIR)/%.cc
	$(CPPCOMPILER) -o $@ -c $<

$(CPP_DEPENDENCIES): $(BIN_DIR)/%.d: $(API_DIR)/%.cpp
	$(CPPCOMPILER) -MM $< | sed -e 1's,^,$(BIN_DIR)/,' > $@

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
