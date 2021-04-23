SOURCE_DIR?=./src
BIN_DIR?=.

# remove the last / if it exits
SOURCE_DIR_NO_SUFFIX=$(shell echo $(SOURCE_DIR) | sed "s/\/$$//g")
BIN_DIR_NO_SUFFIX=$(shell echo $(BIN_DIR) | sed "s/\/$$//g")
BIN_DIR_IN_SED=$(shell echo $(BIN_DIR_NO_SUFFIX)| sed 's/[./]/\\&/g')
SOURCE_DIR_IN_SED=$(shell echo $(SOURCE_DIR_NO_SUFFIX)| sed 's/[./]/\\&/g')

# no prefix
PLAYER_SOURCES= $(shell ls $(SOURCE_DIR_NO_SUFFIX) | grep -E 'player[1-4]?.cpp' |sed ":a;N;s/\n/ /g;ba")
PLAYER_OBJECTS=$(shell echo $(PLAYER_SOURCES)| sed "s/cpp/o/g")

objects=CAPI.o API.o Logic.o main.o Message2Client.pb.o Message2Server.pb.o MessageType.pb.o 

AI_O_PREREQUISITES= ./src/Base.h ./src/Constants.h ./src/Base.h

all: $(objects) player_obj
	for i in $(PLAYER_OBJECTS);\
	do\
		g++ -o  $(BIN_DIR_NO_SUFFIX)/$$(echo $$i| sed "s/.o//g"| sed "s/[a-z]/\u&/g") \
		$$(echo $(objects) | sed -E "s/^| /&$(BIN_DIR_IN_SED)\//g") $(BIN_DIR_NO_SUFFIX)/$$i\
		-L./a -lprotobuf -lhpsocket \
		-Wl,-rpath=./so -std=c++17 -pthread -O2 -Wall;\
	done\
	echo Success

CAPI.o: ./src/CAPI.cpp
	g++ -c ./src/CAPI.cpp -o $(BIN_DIR_NO_SUFFIX)/CAPI.o -I./include/linux -std=c++17 -O2 -Wall

API.o: ./src/API.cpp
	g++ -c ./src/API.cpp -o $(BIN_DIR_NO_SUFFIX)/API.o -I./include/linux -std=c++17 -O2 -Wall

Logic.o: ./src/Logic.cpp
	g++ -c ./src/Logic.cpp -o $(BIN_DIR_NO_SUFFIX)/Logic.o -I./include/linux -pthread -std=c++17 -O2 -Wall

main.o: ./src/main.cpp
	g++ -c ./src/main.cpp -o $(BIN_DIR_NO_SUFFIX)/main.o -I./include/linux -std=c++17 -O2 -Wall


Message2Client.pb.o:./src/proto/Message2Client.pb.h
	g++ -c ./src/proto/Message2Client.pb.cc -o $(BIN_DIR_NO_SUFFIX)/Message2Client.pb.o -I./include/linux -std=c++17 -O2 -Wall

Message2Server.pb.o:./src/proto/Message2Server.pb.h
	g++ -c ./src/proto/Message2Server.pb.cc -o $(BIN_DIR_NO_SUFFIX)/Message2Server.pb.o -I./include/linux -std=c++17 -O2 -Wall

MessageType.pb.o:./src/proto/MessageType.pb.h
	g++ -c ./src/proto/MessageType.pb.cc -o $(BIN_DIR_NO_SUFFIX)/MessageType.pb.o -I./include/linux -std=c++17 -O2 -Wall


.PHONY:clean player_obj

clean:
	-rm $$(echo $(objects) | sed -E "s/^| /&$(BIN_DIR_IN_SED)\//g")\
	 $$(echo $(PLAYER_OBJECTS) | sed -E "s/^| /&$(BIN_DIR_IN_SED)\//g") \
	 $(shell echo $(PLAYER_OBJECTS)| sed "s/.o//g"|sed -E "s/^| /&$(BIN_DIR_IN_SED)\//g"| sed "s/[a-z]/\u&/g")

player_obj: $(shell echo $(PLAYER_SOURCES)|sed -E "s/^| /&$(SOURCE_DIR_IN_SED)\//g")
	for i in $(PLAYER_SOURCES);\
	do\
		g++ -c $(SOURCE_DIR_NO_SUFFIX)/$$i -o $(BIN_DIR_NO_SUFFIX)/$$(echo $$i|sed "s/cpp/o/g")\
		 -I./src/ -std=c++17 -O2 -Wall;\
	done


