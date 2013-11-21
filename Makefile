CXX = clang++
OBJS = obj/main.o obj/file.o obj/renderer.o obj/editor.o
EXECNAME = supvime

default: $(EXECNAME)
	./$(EXECNAME)

$(EXECNAME): $(OBJS)
	$(CXX) -o $@ $^ -lSDL2 -lSDL2_ttf

obj/%.o: ./%.cpp
	$(CXX) -c -o $@ $< -Wall -Wextra -g -std=c++0x

clean:
	-rm -f obj/*.o $(EXECNAME)

