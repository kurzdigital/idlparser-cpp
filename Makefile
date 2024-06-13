BIN = parser
OBJECTS = main.o
CXXFLAGS = -O2 -Wall -Wextra -pedantic -std=c++11

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test: $(BIN)
	for F in samples/*.idl; do \
		./$(BIN) < "$$F" | sort | diff - "$${F%.*}.txt"; done

$(BIN): $(OBJECTS) idlparser.h
	$(CXX) -o $@ $(OBJECTS)

clean:
	rm -f *.o $(BIN)
