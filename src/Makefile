CFLAGS = -g -std=c++11
LDFLAGS = -pthread 
LIB = -ljson-c -lubox -lblobmsg_json -luhttpd -lnl-route-3 -lnl-3 -luuid
INCLUDE = -I ./server/inih/cpp -I$(STAGING_DIR)/usr/include/libubox -I ~/openwrt/staging_dir/host/include/e2fsprogs -I$(STAGING_DIR)/usr/include/uhttpd -I$(STAGING_DIR)/usr/include/libnl3

SRC = ./server/inih/cpp/INIReader.cpp ./server/inih/ini.c ./server/AQService.cpp ./server/Settings.cpp \
 ./server/Session.cpp ./server/TrafficController.cpp ./server/TrafficClassifier.cpp ./server/RuleMac.cpp \
 ./server/RuleFlow.cpp ./server/Neighbour.cpp ./server/Client.cpp ./server/main.cpp ./server/RestServer.cpp \
 ./server/InfoStorage.cpp ./server/Option.cpp

OBJ = $(patsubst %.cpp,%.o,$(SRC))

EXECUTABLE=aqservice

all: $(OBJ) 
	$(CXX) $(CFLAGS) $(SRC) $(LDFLAGS) $(INCLUDE) -o aqservice $(LIB)
	
%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) $< -c

clean:
	rm -f $(wildcard *.o) $(wildcard *.gch) $(EXECUTABLE)
