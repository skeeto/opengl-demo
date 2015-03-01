CFLAGS = -std=c99 -O2 -Wall -Wextra -Igl3w/
LDLIBS = -lglut

main : main.o gl3w/gl3w.o

run : main
	./$^

clean :
	$(RM) main.o gl3w/gl3w.o main main.exe
