CC = g++
CFLAGS = -g -std=c++11
LDFLAGS = -pthread
LIB = -ljson-c -lubox -lblobmsg_json -luhttpd -lnl-route-3 -lnl-3 -luuid
INCLUDE = -I/usr/local/include/json-c -I/usr/local/include/libubox -I/usr/local/include/uhttpd -I/usr/include/libnl3 -I ./inih/cpp

SRC = ./inih/cpp/INIReader.cpp ./inih/ini.c AQService.cpp Settings.cpp Session.cpp TrafficController.cpp TrafficClassifier.cpp RuleMac.cpp RuleFlow.cpp Neighbour.cpp Client.cpp main.cpp RestServer.cpp

OBJ = $(patsubst %.cpp,%.o,$(SRC))

EXECUTABLE=aqservice

all: $(OBJ) 
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) $(INCLUDE) -o aqservice $(LIB)
    
%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $< -c

clean:
	rm -f $(wildcard *.o) $(wildcard *.gch) $(EXECUTABLE)