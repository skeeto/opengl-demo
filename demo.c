#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <getopt.h>

#include <GL/gl3w.h>
#include <GL/freeglut.h>

void gl3wSwapInterval(int x)
{
#ifdef __WIN32__
    BOOL (APIENTRY *wglSwapIntervalEXT)(int) =
        (void *)gl3wGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT)
        wglSwapIntervalEXT(x);
#elif __linux__
    int (*glXSwapIntervalSGI)(int) =
        (void *)gl3wGetProcAddress("glXSwapIntervalSGI");
    if (glXSwapIntervalSGI)
        glXSwapIntervalSGI(x);
#elif __APPLE__
    // TODO: call aglSetInteger()
#else
    (void) x;
#endif
}

#define M_PI 3.141592653589793
#define ATTRIB_POINT 0

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
        fprintf(stderr, "error: %s: %s\n",
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

static void input(unsigned char key, int x, int y)
{
    (void) x;
    (void) y;
    if (key == 27 || key == 'q')
        glutLeaveMainLoop();
}

static void render(void)
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(graphics.program);
    glUniform1f(graphics.uniform_angle, graphics.angle);
    glBindVertexArray(graphics.vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(SQUARE) / sizeof(SQUARE[0]));
    glutSwapBuffers();
    glutPostRedisplay();

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
}

int main(int argc, char *argv[])
{
    /* Options */
    bool fullscreen = false;
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
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_MULTISAMPLE | GLUT_DOUBLE | GLUT_RGBA);
    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    if (fullscreen) {
        int width = glutGet(GLUT_SCREEN_WIDTH);
        int height = glutGet(GLUT_SCREEN_HEIGHT);
        char screenspec[32];
        sprintf(screenspec, "%dx%d:32", width, height);
        glutGameModeString(screenspec);
        glutEnterGameMode();
    } else {
        glutInitWindowSize(640, 640);
        glutCreateWindow("DailyProgrammer");
    }

    /* Initialize gl3w */
    if (gl3wInit()) {
        fprintf(stderr, "gl3w: failed to initialize\n");
        exit(EXIT_FAILURE);
    }
    gl3wSwapInterval(1);

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
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
    graphics.lastframe = usec();
    glutDisplayFunc(render);
    glutKeyboardFunc(input);
    glutMainLoop();
    fprintf(stderr, "Exiting ...\n");

    /* Cleanup and exit */
    glDeleteBuffers(1, &graphics.vbo);
    glDeleteVertexArrays(1, &graphics.vao);
    glDeleteShader(graphics.frag);
    glDeleteShader(graphics.vert);
    glDeleteProgram(graphics.program);
    return 0;
}
