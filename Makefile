CXX = clang++
OBJS = $(EDITOROBJS) $(FILEOBJS) $(RENDOBJS) obj/main.o
EDITOROBJS = obj/editor.o
FILEOBJS = obj/file.o
RENDOBJS = obj/renderer/renderer.o $(REND_WIDGOBJS)
REND_WIDGOBJS = obj/renderer/widgets/texteditor.o
EXECNAME = supvime

default: $(EXECNAME)
	./$(EXECNAME)

$(EXECNAME): $(OBJS)
	$(CXX) -o $@ $^ -lSDL2 -lSDL2_ttf

obj/%.o: ./%.cpp
	$(CXX) -c -o $@ $< -Wall -Wextra -g -std=c++0x

clean:
	-rm -f obj/*.o obj/renderer/*.o obj/renderer/widgets/*.o $(EXECNAME)

