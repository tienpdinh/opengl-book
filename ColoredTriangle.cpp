#include "glad/glad.h"
#if defined(__APPLE__) || defined(__linux__)
  #include<SDL2/SDL.h>
  #include<SDL2/SDL_opengl.h>
#else
  #include<SDL.h>
  #include<SDL_opengl.h>
#endif

// 2D vertex shader
const GLchar *vertexSource =
  "#version 150 core\n"
  "in vec2 position;"
  "in vec3 inColor;"
  "out vec3 Color;"
  "void main() {"
  "  Color = inColor;"
  "  gl_Position = vec4(position, 0.0, 1.0);"
  "}";

// basic fragment shader
const GLchar *fragmentSource = 
  "#version 150 core\n"
  "in vec3 Color;"
  "out vec4 outColor;"
  "void main() {"
  "  outColor = vec4(Color, 1.0);" // RGB alpha
  "}";

// globals
bool fullScreen = false;
int screenWidth = 800;
int screenHeight = 600;

void loadShader(GLuint shaderID, const GLchar *shaderSource) {
  glShaderSource(shaderID, 1, &shaderSource, NULL);
  glCompileShader(shaderID);

  // double check shader compiled
  GLint status;
  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
  if (!status) {
    char buffer[512]; glGetShaderInfoLog(shaderID, 512, NULL, buffer);
    printf("Shader Compile Failed. Info:\n\n%s\n", buffer);
  }
}

int main(int argc, char **argv) {
  // initialize SDL
  SDL_Init(SDL_INIT_VIDEO);

  // Print version of SDL we are using
  SDL_version comp; SDL_version linked;
  SDL_VERSION(&comp); SDL_GetVersion(&linked);
  printf("\nCompiled against SDL version %d.%d.%d\n", comp.major, comp.minor, comp.patch);
  printf("Linked SDL version %d.%d.%d\n", linked.major, linked.minor, linked.patch);

  // Ask SDL to get a recent version of OpenGL (3.2 or greater)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  // Create a window (offsetx, offsety, width, height, flags)
  SDL_Window *window = SDL_CreateWindow("My OpenGL Program", 100, 100,
                                        screenWidth, screenHeight, SDL_WINDOW_OPENGL);
  if (!window) {
    printf("Could not create window: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }
  float aspect = screenWidth/(float)screenHeight;

  SDL_GLContext context = SDL_GL_CreateContext(window); // Bind OpenGL to the window

  // initialize opengl through glad
  if (gladLoadGLLoader(SDL_GL_GetProcAddress)) {
    printf("OpenGL loaded\n");
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));
  } else {
    printf("ERROR: Failed to initialize OpenGL context.\n");
    return -1;
  }

  // load models and shaders
  // load and compile shaders
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  loadShader(vertexShader, vertexSource);
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  loadShader(fragmentShader, fragmentSource);

  // join vertex and fragment shader to one
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glBindFragDataLocation(shaderProgram, 0, "outColor");
  glLinkProgram(shaderProgram);

  // load model data
  GLfloat vertices[] = {
    0.0f, 0.5f, 1.0f, 0.0f, 0.0f,    // Vertex 1: postion = (0,.5) color = Red
    0.5f, -0.5f, 0.0f, 1.0f, 0.0f,   // Vertex 2: postion = (.5,-.5) color = Green
    -0.5f, -0.5f, 0.0f, 0.0f, 1.0f}; // Vertex 3: postion = (-.5,-.5) color = Blue

  // create VBO
  GLuint vbo;
  glGenBuffers(1, &vbo);  // create 1 buffer called vbo
  glBindBuffer(GL_ARRAY_BUFFER, vbo); // only one buffer can be bounded at a time
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // create VAO
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // bind attributes to VBO
  GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
  glEnableVertexAttribArray(posAttrib);

  GLint colAttrib = glGetAttribLocation(shaderProgram, "inColor");
  glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
  glEnableVertexAttribArray(colAttrib);
  glBindVertexArray(0);

  // main game loop
  SDL_Event windowEvent;
  bool quit = false;
  while (!quit) {
    // process keyboard and mouse events
    while (SDL_PollEvent(&windowEvent)) {
      if (windowEvent.type == SDL_QUIT) quit = true;
      if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
        quit = true;
      if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) {
        fullScreen = !fullScreen;
        SDL_SetWindowFullscreen(window, fullScreen ? SDL_WINDOW_FULLSCREEN : 0);
      }
    }

    // draw content
    glClearColor(.2f, .4f, .8f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    SDL_GL_SwapWindow(window);
  }

  // clean up
  glDeleteProgram(shaderProgram);
  glDeleteShader(fragmentShader);
  glDeleteShader(vertexShader);
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
  SDL_GL_DeleteContext(context);
  SDL_Quit();

  return 0;
}