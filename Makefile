CXX=g++
CXXFLAGS=-Wall -Wextra -Werror -pedantic -ansi -g -msse
INCLUDE=-I/home/matthias/.local/include/
VOICE=cmu_us_kal
LDFLAGS=-lflite_$(VOICE) -lflite_usenglish -lflite_cmulex -lflite -lm -lalut -lcurl -ljson_linux-gcc-4.3.4_libmt -lboost_thread

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
