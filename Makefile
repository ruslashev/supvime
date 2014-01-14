CXX = clang++

OBJS = $(EDITOROBJS) $(FILEOBJS) $(RENDOBJS) $(MISCOBJS) obj/main.o
EDITOROBJS = obj/editor.o
FILEOBJS = obj/file.o
RENDOBJS = obj/renderer/renderer.o $(REND_WIDGOBJS)
REND_WIDGOBJS = obj/renderer/widgets/texteditor.o
MISCOBJS = obj/errors.o obj/glutils.o

EXECNAME = supvime

default: objdir $(EXECNAME)
	./$(EXECNAME)

$(EXECNAME): $(OBJS)
	$(CXX) -o $@ $^ -lGL -lGLEW -lSDL2 -lfreetype

obj/%.o: source/%.cpp
	$(CXX) -c -o $@ $< -Wall -Wextra -g -std=c++0x `freetype-config --cflags`

objdir:
	mkdir -p obj
	mkdir -p obj/renderer
	mkdir -p obj/renderer/widgets

clean:
	-rm -f $(OBJS) $(EXECNAME)

