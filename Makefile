CXX = clang++
CC  = clang

OBJS = $(EDITOROBJS) $(FILEOBJS) $(RENDOBJS) obj/main.o
EDITOROBJS = obj/editor.o
FILEOBJS = obj/file.o
RENDOBJS = obj/renderer/renderer.o $(REND_WIDGOBJS)
REND_WIDGOBJS = obj/renderer/widgets/texteditor.o

EXECNAME = supvime

default: $(EXECNAME)
	./$(EXECNAME)

$(EXECNAME): $(OBJS)
	$(CXX) -o $@ $^ -lGL -lGLEW -lSDL2

obj/%.o: ./%.cpp
	$(CXX) -c -o $@ $< -Wall -Wextra -g -std=c++0x

obj/%.o: ./%.c
	$(CC) -c -o $@ $< -Wall -Wextra -g

clean:
	-rm -f obj/*.o obj/renderer/*.o obj/renderer/widgets/*.o
	-rm -f $(EXECNAME)

