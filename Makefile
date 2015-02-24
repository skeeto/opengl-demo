CFLAGS = -std=c99 -O2 -Wall -Wextra -pedantic \
   -Iglew/ -DGLEW_STATIC -DGLEW_NO_GLU
LDLIBS = -lSDL2 -lGL

main : main.o glew/glew.o

run : main
	./$^

clean :
	$(RM) main.o glew/glew.o main main.exe
