#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#include <SDL2/SDL.h>
#include <GL/glew.h>

static uint64_t usec(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

static GLuint compile_shader(GLenum type, const GLchar *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint param;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &param);
    if (!param) {
        GLchar log[4096];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        printf("error: %s: %s\n",
               GL_FRAGMENT_SHADER ? "frag" : "vert", (char *) log);
        exit(EXIT_FAILURE);
    }
    return shader;
}

static GLuint link_program(GLuint vert, GLuint frag)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    GLint param;
    glGetProgramiv(program, GL_LINK_STATUS, &param);
    if (!param) {
        GLchar log[4096];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        printf("error: link: %s\n", (char *) log);
        exit(EXIT_FAILURE);
    }
    return program;
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    /* Create window and OpenGL context */
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window =
        SDL_CreateWindow("DailyProgrammer",
                         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         640, 640, SDL_WINDOW_OPENGL);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        printf("error: glew: %s\n", glewGetErrorString(glew_status));
        exit(EXIT_FAILURE);
    }
    SDL_GL_SetSwapInterval(1);

    /* Shader sources */
    const GLchar *vert_shader =
        "attribute vec2 point;\n"
        "uniform float angle;\n"
        "void main() {\n"
        "    mat2 rotate = mat2(cos(angle), -sin(angle),\n"
        "                       sin(angle), cos(angle));\n"
        "    gl_Position = vec4(rotate * point, 0.0, 1.0);\n"
        "}\n";
    const GLchar *frag_shader =
        "void main() {\n"
        "    gl_FragColor = vec4(1, 0, 0, 0);\n"
        "}\n";

    /* Compile and link OpenGL program */
    GLuint vert = compile_shader(GL_VERTEX_SHADER, vert_shader);
    GLuint frag = compile_shader(GL_FRAGMENT_SHADER, frag_shader);
    GLuint program = link_program(vert, frag);
    GLint attrib_point = glGetAttribLocation(program, "point");
    GLint uniform_angle = glGetUniformLocation(program, "angle");

    /* Prepare vertex buffer */
    GLuint vert_buffer;
    glGenBuffers(1, &vert_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vert_buffer);
    float square[] = {-0.75, 0.75, -0.75, -0.75, 0.75, 0.75, 0.75, -0.75};
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);

    int running = 1;
    float angle = 0.0f;
    int count = 0;
    uint64_t last = usec();
    while (running) {
        /* Clear the screen and draw the square */
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        glEnableVertexAttribArray(attrib_point);
        glVertexAttribPointer(attrib_point, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glUniform1f(uniform_angle, angle += 0.01f);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(square) / sizeof(square[0]));
        SDL_GL_SwapWindow(window);

        /* Exit on window close */
        SDL_Event event;
        while (SDL_PollEvent(&event))
            if (event.type == SDL_QUIT)
                running = 0;

        /* FPS counter */
        count++;
        if (usec() - last > 1000000ULL) {
            printf("FPS: %d\n", count);
            count = 0;
            last = usec();
        }
    }


    /* Cleanup and exit */
    glDeleteBuffers(1, &vert_buffer);
    glDeleteShader(frag);
    glDeleteShader(vert);
    glDeleteProgram(program);
    SDL_GL_DeleteContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
