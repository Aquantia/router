CFLAGS = -g -std=c++11
LDFLAGS = -pthread 
LIB = -ljson-c -lubox -lblobmsg_json -luhttpd -lnl-route-3 -lnl-3 -luuid
INCLUDE = -I ./inih/cpp -I$(STAGING_DIR)/usr/include/libubox -I ~/openwrt/staging_dir/host/include/e2fsprogs -I$(STAGING_DIR)/usr/include/uhttpd -I$(STAGING_DIR)/usr/include/libnl3

SRC = ./inih/cpp/INIReader.cpp ./inih/ini.c ./AQService.cpp ./Settings.cpp \
 ./Session.cpp ./TrafficController.cpp ./TrafficClassifier.cpp ./RuleMac.cpp \
 ./RuleFlow.cpp ./Neighbour.cpp ./Client.cpp ./main.cpp ./RestServer.cpp \
 ./InfoStorage.cpp ./Option.cpp

OBJ = $(patsubst %.cpp,%.o,$(SRC))

EXECUTABLE=aqservice

all: $(OBJ) 
	$(CXX) $(CFLAGS) $(SRC) $(LDFLAGS) $(INCLUDE) -o aqservice $(LIB)
	
%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) $< -c

clean:
	rm -f $(wildcard *.o) $(wildcard *.gch) $(EXECUTABLE)
