# OpenGL 3.3 Core Profile Demo

This is a demo showing a minimal, portable OpenGL 3.3 Core Profile
setup. The OpenGL functions are loaded with [gl3w][gl3w] and the
context is created using [GLFW][glfw]. Linux, Windows, OS X, and the
BSDs are supported.

Read more: [Minimal OpenGL 3.3 Core Profile Demo][more]

### Linux

You'll need GLFW. On any Debian-based system:

    # apt-get install libglfw3-dev

If your GPU doesn't support OpenGL 3.3 and you're using Mesa 10.0+,
you can force software rendering with the environment variable
`LIBGL_ALWAYS_SOFTWARE=1`. The Makefile has a `run-on-mesa` target
to set this for you.


### Windows

The Windows build requires the wonderful [MinGW-w64][mingw], and will
be statically compiled against GLFW to create a fully standalone
application. Build using `make -f Makefile.mingw`.

### OSX

    # brew install glfw3

Build the application using `make -f Makefile.osx`.

### OSX

    # brew install glfw3

Build the application using "make -f Makefile.osx"

## Alternatives

A good alternative to gl3w would be [glLoadGen][glloadgen], which
could *almost* be dropped directly in-place. [GLEW][glew] is both much
larger and does not support core profile or 3.2+ in general, so it's
not an option for modern OpenGL.

A reasonable alternative to GLFW is [SDL 2.0][sdl], which is bigger
and brings along a lot more functionality, such as audio, threading,
and font rendering, should you need it.


[gl3w]: https://github.com/skaslev/gl3w
[mingw]: http://mingw-w64.sourceforge.net/
[glloadgen]: https://bitbucket.org/alfonse/glloadgen/wiki/Home
[glew]: http://glew.sourceforge.net/
[glfw]: http://www.glfw.org/
[sdl]: https://www.libsdl.org/
[more]: http://nullprogram.com/blog/2015/06/06/
