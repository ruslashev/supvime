#include <fstream>
#include "file.hpp"
#include "renderer.hpp"
#include "editor.hpp"

/*         //////                                  //
 *      //        //    //  //////    //      //      //////  ////      ////
 *       ////    //    //  //    //  //      //  //  //    //    //  ////////
 *          //  //    //  //    //    //  //    //  //    //    //  //
 *   //////      //////  //////        //      //  //    //    //    //////
 *                      //
 *                     //
 *
 *
 *  Main component        Must provide
 *  processes keys,      'char getch()'
 *  edits the file         function
 *    +--------+         +----------+
 *    | Editor | <------ | Renderer | _
 *    +--------+         +----------+   \ std::vector of
 *        |                              \
 *        | 1 or more                +--------------------+
 *        | someday there will       | BaseDrawableWidget |
 *        | be std::vector           +--------------------+
 *        v                                    |
 *     +------+                                ^
 *     | File |                              /   \
 *     +------+                +------------+     (in future)
 *       aka                   | TextEditor |     * status bar
 *     'buffer'                +------------+     * tab bar
 *      in vim                                    * :command line bar
 *
 */


int main()
{
	File file("file.cpp");
	Editor ed(&file);
	Renderer rend(&ed);
	rend.Update(file.lines);

	while (1) {
		rend.Update(file.lines);
		ed.ProcessKey(rend.getch());
	}

	return 0;
}

