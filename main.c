#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#include <GL/glew.h>
#ifdef __WIN32__
#include <GL/wglew.h>
#define glutSwapInterval(x) wglSwapIntervalEXT(x)
#elif __linux__
#include <GL/glxew.h>
#define glutSwapInterval(x) \
    glXSwapIntervalEXT(glXGetCurrentDisplay(), glXGetCurrentDrawable(), x)
#endif
#include <GL/freeglut.h>

#define M_PI 3.141592653589793

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

struct {
    int fps;
    GLuint vert;
    GLuint frag;
    GLuint program;
    GLint attrib_point;
    GLint uniform_angle;
    GLuint vert_buffer;
    float angle;
    uint32_t framecount;
    uint64_t lastframe;
} graphics = {30};

const float SQUARE[] = {
    -1.0f,  1.0f,
    -1.0f, -1.0f,
     1.0f,  1.0f,
     1.0f, -1.0f
};

static void render(void)
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(graphics.program);
    glEnableVertexAttribArray(graphics.attrib_point);
    glVertexAttribPointer(graphics.attrib_point, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glUniform1f(graphics.uniform_angle, graphics.angle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(SQUARE) / sizeof(SQUARE[0]));
    glutSwapBuffers();
    glutPostRedisplay();
    glutSwapInterval(1);

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
    /* Create window and OpenGL context */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitContextVersion(2, 1);
    /* char screenspec[32]; */
    /* int width = glutGet(GLUT_SCREEN_WIDTH); */
    /* int height = glutGet(GLUT_SCREEN_HEIGHT); */
    /* sprintf(screenspec, "%dx%d:32", width, height); */
    /* glutGameModeString(screenspec); */
    /* glutEnterGameMode(); */
    glutInitWindowSize(640, 640);
    glutCreateWindow("DailyProgrammer");

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        printf("error: glew: %s\n", glewGetErrorString(glew_status));
        exit(EXIT_FAILURE);
    }

    /* Shader sources */
    const GLchar *vert_shader =
        "attribute vec2 point;\n"
        "uniform float angle;\n"
        "void main() {\n"
        "    mat2 rotate = mat2(cos(angle), -sin(angle),\n"
        "                       sin(angle), cos(angle));\n"
        "    gl_Position = vec4(0.75 * rotate * point, 0.0, 1.0);\n"
        "}\n";
    const GLchar *frag_shader =
        "void main() {\n"
        "    gl_FragColor = vec4(1, 0, 0, 0);\n"
        "}\n";

    /* Compile and link OpenGL program */
    graphics.vert = compile_shader(GL_VERTEX_SHADER, vert_shader);
    graphics.frag = compile_shader(GL_FRAGMENT_SHADER, frag_shader);
    graphics.program = link_program(graphics.vert, graphics.frag);
    graphics.attrib_point = glGetAttribLocation(graphics.program, "point");
    graphics.uniform_angle = glGetUniformLocation(graphics.program, "angle");

    /* Prepare vertex buffer */
    glGenBuffers(1, &graphics.vert_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, graphics.vert_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SQUARE), SQUARE, GL_STATIC_DRAW);

    /* Start main loop */
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
    graphics.lastframe = usec();
    glutDisplayFunc(render);
    glutMainLoop();
    printf("Exiting ...");

    /* Cleanup and exit */
    glDeleteBuffers(1, &graphics.vert_buffer);
    glDeleteShader(graphics.frag);
    glDeleteShader(graphics.vert);
    glDeleteProgram(graphics.program);
    return 0;
}
