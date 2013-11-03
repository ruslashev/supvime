CXX = clang++
CXXFLAGS = -Wall -Wextra -g -std=c++0x
OBJS = obj/main.o obj/file.o
EXECNAME = supvime

default: $(EXECNAME)
	./$(EXECNAME)

$(EXECNAME): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

obj/%.o: ./%.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
	-rm -f obj/*.o $(EXECNAME)

