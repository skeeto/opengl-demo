CFLAGS = -std=c99 -O2 -Wall -Wextra -Igl3w/
LDLIBS = -lglfw -lGL -ldl

demo : demo.c gl3w/gl3w.c

run : demo
	./$^

run-on-mesa : demo
	export LIBGL_ALWAYS_SOFTWARE=1
	./$^

clean :
	$(RM) demo demo.exe
