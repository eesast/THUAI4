PLAYER_SOURCES= $(shell ls ./src/ | grep -E 'AI[1-4]?.cpp' |sed ":a;N;s/\n/ /g;ba")
PLAYER_OBJECTS=$(shell echo $(PLAYER_SOURCES)| sed "s/cpp/o/g")

objects=CAPI.o API.o Logic.o main.o Message2Client.pb.o Message2Server.pb.o MessageType.pb.o

AI_O_PREREQUISITES= ./src/Base.h ./src/Constants.h ./src/Base.h

CAPI: $(objects) player_obj
	for i in $(PLAYER_OBJECTS);\
	do\
		g++ -o  $$(echo $$i| sed "s/.o//g") $(objects) $$i  -L./a -lprotobuf -lhpsocket \
		-Wl,-rpath=./so -std=c++17 -pthread -O2 -Wall;\
	done

CAPI.o: 
	g++ -c ./src/CAPI.cpp -I./include/linux -std=c++17 -O2 -Wall

API.o: 
	g++ -c ./src/API.cpp -I./include/linux -std=c++17 -O2 -Wall

Logic.o: 
	g++ -c ./src/Logic.cpp -I./include/linux -pthread -std=c++17 -O2 -Wall

main.o: 
	g++ -c ./src/main.cpp -I./include/linux -std=c++17 -O2 -Wall


Message2Client.pb.o:./src/proto/Message2Client.pb.h
	g++ -c ./src/proto/Message2Client.pb.cc -I./include/linux -std=c++17 -O2 -Wall

Message2Server.pb.o:./src/proto/Message2Server.pb.h
	g++ -c ./src/proto/Message2Server.pb.cc -I./include/linux -std=c++17 -O2 -Wall

MessageType.pb.o:./src/proto/MessageType.pb.h
	g++ -c ./src/proto/MessageType.pb.cc -I./include/linux -std=c++17 -O2 -Wall


.PHONY:clean player_obj

clean:
	-rm $(objects) $(PLAYER_OBJECTS) $(shell echo $(PLAYER_OBJECTS)| sed "s/.o//g")

player_obj: $(shell echo $(PLAYER_SOURCES)|sed -E "s/^| /&\.\/src\//g")
	for i in $(PLAYER_SOURCES);\
	do\
		g++ -c ./src/$$i -std=c++17 -O2 -Wall;\
	done


