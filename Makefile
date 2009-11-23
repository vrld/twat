# EDIT THESE TO POINT TO JSONCPP!
JSONCPP_INCLUDE_DIR=$(HOME)/.local/include/
JSONCPP_LIBRARY=json_linux-gcc-4.3.4_libmt

# CHANGE THE VOICE HERE
VOICE=cmu_us_kal

# this should be fine
CXX=g++
CXXFLAGS=-Wall -Wextra -Werror -pedantic -ansi -g -msse -D VOICE=register_$(VOICE) -I$(JSONCPP_INCLUDE_DIR)
LDFLAGS=-lflite_$(VOICE) -lflite_usenglish -lflite_cmulex -lflite -lm -lalut -lcurl -lboost_thread -l$(JSONCPP_LIBRARY)

SOURCES=twatter.cpp twat.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=twatter

all: $(SOURCES) $(EXECUTABLE) 

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCLUDE) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)
