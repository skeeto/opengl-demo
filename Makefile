CFLAGS = -std=c99 -O2 -Wall -pedantic \
   -Iglew/ -DGLEW_STATIC -DGLEW_NO_GLU
LDLIBS = -lGL -lglut

main : main.o glew/glew.o

run : main
	./$^

clean :
	$(RM) main.o glew/glew.o main main.exe
