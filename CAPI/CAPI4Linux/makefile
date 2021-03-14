objects=CAPI.o API.o AI.o Logic.o main.o Message2Client.pb.o Message2Server.pb.o MessageType.pb.o

CAPI_O_PREREQUISITES= ./src/CAPI.h 

API_O_PREREQUISITES= ./src/Structures.h ./src/Constants.h  ./src/Base.h ./src/API.h

LOGIC_O_PREREQUISITES= ./src/Logic.h $(CAPI_O_PREREQUISITES) $(API_O_PREREQUISITES)

AI_O_PREREQUISITES= ./src/AI.h ./src/Base.h ./src/Constants.h ./src/Base.h

MAIN_O_PREREQUISITES= $(LOGIC_O_PREREQUISITES) $(AI_O_PREREQUISITES)

CAPI: $(objects)
	g++ -o CAPI $(objects) -L./a -lprotobuf -lhpsocket \
	-Wl,-rpath=./so -std=c++17 -pthread -O2 -Wall

CAPI.o: $(CAPI_O_PREREQUISITES)
	g++ -c ./src/CAPI.cpp -I./include -std=c++17 -O2 -Wall

API.o: $(API_O_PREREQUISITES)
	g++ -c ./src/API.cpp -I./include -std=c++17 -O2 -Wall

Logic.o: $(LOGIC_O_PREREQUISITES)
	g++ -c ./src/Logic.cpp -I./include -pthread -std=c++17 -O2 -Wall

main.o: $(MAIN_O_PREREQUISITES)
	g++ -c ./src/main.cpp -I./include -std=c++17 -O2 -Wall

AI.o: $(AI_O_PREREQUISITES)
	g++ -c ./src/AI.cpp -std=c++17 -O2 -Wall

Message2Client.pb.o:./src/proto/Message2Client.pb.h
	g++ -c ./src/proto/Message2Client.pb.cc -std=c++17 -O2 -Wall

Message2Server.pb.o:./src/proto/Message2Server.pb.h
	g++ -c ./src/proto/Message2Server.pb.cc -std=c++17 -O2 -Wall

MessageType.pb.o:./src/proto/MessageType.pb.h
	g++ -c ./src/proto/MessageType.pb.cc -std=c++17 -O2 -Wall

.PHONY:clean

clean:
	-rm CAPI $(objects)


