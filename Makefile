CFLAGS = -std=c99 -O2 -Wall -Wextra -Igl3w/
LDLIBS = -lglfw3 -ldl -framework OpenGL 

demo : demo.c gl3w/gl3w.c

run : demo
	./$^

clean :
	$(RM) demo demo.exe
