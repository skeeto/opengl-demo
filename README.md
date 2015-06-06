# OpenGL 3.3 Core Profile Demo

This is a demo showing a minimal, portable OpenGL 3.3 Core Profile
setup. The OpenGL functions are loaded with [gl3w][gl3w] and the
context is created using [FreeGLUT][glut]. Both Linux and Windows are
directly supported. In theory OS X could also be supported but I don't
own any Apple hardware to test it on.

An attempt is made to enable vsync (swap interval = 1) by calling
platform-specific extensions (see `gl3wSwapInterval`) when available.

Read more: [Minimal OpenGL 3.3 Core Profile Demo][more]

### Linux

You'll need to install FreeGLUT. On any Debian-based system:

    # apt-get install freeglut3-dev

### Windows

The Windows build requires the wonderful [MinGW-w64][mingw], and will
be statically compiled against [this particular FreeGLUT binary
distribution][bin] to create a fully standalone application. Unzip the
downloaded zip file directly at the root of the repository and it's
ready to build.

## Alternatives

A good alternative to gl3w would be [glLoadGen][glloadgen], which
could *almost* be dropped directly in-place. [GLEW][glew] is both much
larger and does not support core profile or 3.2+ in general, so it's
not an option for modern OpenGL.

A good alternative to FreeGLUT is [GLFW][glfw], which is well-scoped
and, unlike GLUT, isn't loaded down with mostly outdated (useless in
core profile) functions. Another good choice, if you don't mind the
size, is [SDL 2.0][sdl]. Someday I'd like to see an ultra-lightweight,
embeddable windowing library, to complement the OpenGL loader, so that
a portable OpenGL project, like this one, can be absolutely
standalone.


[gl3w]: https://github.com/skaslev/gl3w
[glut]: http://freeglut.sourceforge.net/
[mingw]: http://mingw-w64.sourceforge.net/
[bin]: http://www.transmissionzero.co.uk/software/freeglut-devel/
[glloadgen]: https://bitbucket.org/alfonse/glloadgen/wiki/Home
[glew]: http://glew.sourceforge.net/
[glfw]: http://www.glfw.org/
[sdl]: https://www.libsdl.org/
[more]: http://nullprogram.com/blog/2015/06/06/
