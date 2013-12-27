# Supvime

***Supvime*** is a project set to create the world's best text editor.

That is, it is:

1. Awesome and powerful as Vim
2. Beatiful as Sublime Text
3. Is better than Gvim
4. Is Supreme

### Technical Documentation

Here's rough draft of how it works inside:

           //////                                  //
        //        //    //  //////    //      //      //////  ////      ////
         ////    //    //  //    //  //      //  //  //    //    //  ////////
            //  //    //  //    //    //  //    //  //    //    //  //
     //////      //////  //////        //      //  //    //    //    //////
                        //
                       //

               dependencies
    Main component   |    Must provide
    processes keys,  /   'char getch()'
    edits the file  |      function
      +--------+    v    +----------+
      | Editor | <------ | Renderer | _
      +--------+         +----------+   \ std::vector of
          |                              \
          | 1 or more                +--------------------+
          | someday there will       | BaseDrawableWidget |
          | be std::vector           +--------------------+
          v                                    |
       +------+                                ^
       | File |                              /   \
       +------+                +------------+     (in future)
         aka                   | TextEditor |     * status bar
       'buffer'                +------------+     * tab bar
        in vim                                    * :command line bar

### Libraries

Supvime wouldn't exist without the following libraries:

* [SDL 2](http://libsdl.org/)
* [Font stash](https://github.com/akrinke/Font-Stash)

