#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <getopt.h>

#define GLFW_INCLUDE_NONE
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#define countof(x) (sizeof(x) / sizeof(0[x]))

#define M_PI 3.141592653589793
#define ATTRIB_POINT 0

static uint64_t
usec(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

static GLuint
compile_shader(GLenum type, const GLchar *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint param;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &param);
    if (!param) {
        GLchar log[4096];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "error: %s: %s\n",
                type == GL_FRAGMENT_SHADER ? "frag" : "vert", (char *) log);
        exit(EXIT_FAILURE);
    }
    return shader;
}

static GLuint
link_program(GLuint vert, GLuint frag)
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
        fprintf(stderr, "error: link: %s\n", (char *) log);
        exit(EXIT_FAILURE);
    }
    return program;
}

struct {
    GLuint vert;
    GLuint frag;
    GLuint program;
    GLint uniform_angle;
    GLuint vbo;
    GLuint vao;
    float angle;
    uint32_t framecount;
    uint64_t lastframe;
} graphics;

const float SQUARE[] = {
    -1.0f,  1.0f,
    -1.0f, -1.0f,
     1.0f,  1.0f,
     1.0f, -1.0f
};

static void
render(GLFWwindow *window)
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(graphics.program);
    glUniform1f(graphics.uniform_angle, graphics.angle);
    glBindVertexArray(graphics.vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, countof(SQUARE));

    /* Physics */
    uint64_t now = usec();
    uint64_t udiff = now - graphics.lastframe;
    graphics.angle += 0.000001f * udiff;
    if (graphics.angle > M_PI)
        graphics.angle -= M_PI;
    graphics.framecount++;
    if (now / 1000000 != graphics.lastframe / 1000000) {
        printf("FPS: %d\n", graphics.framecount);
        graphics.framecount = 0;
    }
    graphics.lastframe = now;

    glfwSwapBuffers(window);
}

void
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void) scancode;
    (void) mods;
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

int
main(int argc, char **argv)
{
    /* Options */
    bool fullscreen = false;
    const char *title = "OpenGL 3.3 Demo";

    int opt;
    while ((opt = getopt(argc, argv, "f")) != -1) {
        switch (opt) {
            case 'f':
                fullscreen = true;
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    /* Create window and OpenGL context */
    GLFWwindow *window;
    if (!glfwInit()) {
        fprintf(stderr, "GLFW3: failed to initialize\n");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if (fullscreen) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *m = glfwGetVideoMode(monitor);
        window = glfwCreateWindow(m->width, m->height, title, monitor, NULL);
    } else {
        window = glfwCreateWindow(640, 640, title, NULL, NULL);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    /* Initialize gl3w */
    if (gl3wInit()) {
        fprintf(stderr, "gl3w: failed to initialize\n");
        exit(EXIT_FAILURE);
    }

    /* Shader sources */
    const GLchar *vert_shader =
        "#version 330\n"
        "layout(location = 0) in vec2 point;\n"
        "uniform float angle;\n"
        "void main() {\n"
        "    mat2 rotate = mat2(cos(angle), -sin(angle),\n"
        "                       sin(angle), cos(angle));\n"
        "    gl_Position = vec4(0.75 * rotate * point, 0.0, 1.0);\n"
        "}\n";
    const GLchar *frag_shader =
        "#version 330\n"
        "void main() {\n"
        "    gl_FragColor = vec4(1, 0, 0, 0);\n"
        "}\n";

    /* Compile and link OpenGL program */
    graphics.vert = compile_shader(GL_VERTEX_SHADER, vert_shader);
    graphics.frag = compile_shader(GL_FRAGMENT_SHADER, frag_shader);
    graphics.program = link_program(graphics.vert, graphics.frag);
    graphics.uniform_angle = glGetUniformLocation(graphics.program, "angle");
    glDeleteShader(graphics.frag);
    glDeleteShader(graphics.vert);

    /* Prepare vertex buffer object (VBO) */
    glGenBuffers(1, &graphics.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, graphics.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SQUARE), SQUARE, GL_STATIC_DRAW);

    /* Prepare vertrex array object (VAO) */
    glGenVertexArrays(1, &graphics.vao);
    glBindVertexArray(graphics.vao);
    glVertexAttribPointer(ATTRIB_POINT, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(ATTRIB_POINT);
    glBindVertexArray(0);

    /* Start main loop */
    glfwSetKeyCallback(window, key_callback);
    graphics.lastframe = usec();
    while (!glfwWindowShouldClose(window)) {
        render(window);
        glfwPollEvents();
    }
    fprintf(stderr, "Exiting ...\n");

    /* Cleanup and exit */
    glDeleteBuffers(1, &graphics.vbo);
    glDeleteVertexArrays(1, &graphics.vao);
    glDeleteProgram(graphics.program);

    glfwTerminate();
    return 0;
}
