#include <cstdlib>
#include <iostream>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "core/Camera.hpp"
#include "core/RenderInfo.hpp"
#include "core/ResourcePool.hpp"
#include "core/VoxelModel.hpp"

constexpr int WINDOW_DEFAULT_W = 800;
constexpr int WINDOW_DEFAULT_H = 600;

using namespace std;

int
main(int argc, char const* argv[])
{
  int    argIndex       = 1;
  string surfaceTexture = "res/tile1.jpg";
  string reliefTexture  = "res/tile1.png";
  if (argIndex + 1 < argc && argv[argIndex] == "-t"s) {
    string texturePatternName = argv[argIndex + 1];
    argIndex += 2;

    surfaceTexture = "res/" + texturePatternName + ".jpg";
    reliefTexture  = "res/" + texturePatternName + ".png";
  }

  if (argIndex < argc) {
    cerr << "Invalid or Incorrect param " << argv[argIndex] << endl;
    return EXIT_FAILURE;
  }

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    cerr << "Can not initialize SDL2 " << SDL_GetError() << endl;
    return EXIT_FAILURE;
  }
  atexit(SDL_Quit);

  if (TTF_Init() < 0) {
    cerr << "Can not initialize SDL2_ttf" << SDL_GetError() << endl;
    return EXIT_FAILURE;
  }
  atexit(TTF_Quit);

  // Set the OpenGL properties here!
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_Window* window = SDL_CreateWindow("BedTest",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        WINDOW_DEFAULT_W,
                                        WINDOW_DEFAULT_H,
                                        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
  if (window == nullptr) {
    cerr << "Can not create window SDL2 " << SDL_GetError() << endl;
    return EXIT_FAILURE;
  }

  SDL_GL_CreateContext(window);
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    /* Problem: glewInit failed, something is seriously wrong. */
    cerr << "Error: " << glewGetErrorString(err) << endl;
    return 1;
  }
  cout << "Status: Using GLEW" << glewGetString(GLEW_VERSION) << endl;

  // Use Vsync
  if (SDL_GL_SetSwapInterval(1) < 0) {
    cerr << "Warning: Unable to set VSync! SDL Error: %s\n"
         << SDL_GetError() << endl;
  }

  // Settings
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glViewport(0, 0, WINDOW_DEFAULT_W, WINDOW_DEFAULT_H);
  glClearColor(.25f, .65f, .999f, 1.f);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  BasicCamera  camera(90.f, 0.f, glm::vec3(0.0f, -2.0f, 0.0f));
  VoxelModel   voxel;
  ResourcePool resourcePool;
  RenderInfo   renderInfo;
  renderInfo.shaderProgram  = resourcePool.getShaderProgram("relief");
  renderInfo.surfaceTexture = resourcePool.getTexture(surfaceTexture);
  renderInfo.reliefTexture  = resourcePool.getTexture(reliefTexture);
  renderInfo.faceBitSet     = 0xff;

  float fov              = 45.f;
  float cameraSpeed      = 2.5f; // adjust accordingly
  float rotationSpeed    = 72.f; // adjust accordingly
  float mouseSensitivity = 0.05f;
  float rotationX        = 0;
  float rotationY        = 0;

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);

  auto  keys        = SDL_GetKeyboardState(nullptr);
  float currentTime = SDL_GetTicks(), lastTime = currentTime, delta = 0;

  for (;;) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
        case SDL_QUIT:
          return EXIT_SUCCESS;
        case SDL_KEYDOWN:
          switch (ev.key.keysym.scancode) {
            case SDL_SCANCODE_F2:
              camera.rotateTo(-90, 0);
              camera.position(glm::vec3(0.0f, 0.0f, 3.0f));
              break;
            case SDL_SCANCODE_F3:
              rotationX = rotationY = 0;
            default:
              break;
          }
          break;
        case SDL_MOUSEMOTION:
          camera.rotateBy(ev.motion.xrel * mouseSensitivity,
                          -ev.motion.yrel * mouseSensitivity);
          break;
        case SDL_MOUSEWHEEL:
          if (fov >= 1.0f && fov <= 45.0f)
            fov -= ev.wheel.y;
          if (fov <= 1.0f)
            fov = 1.0f;
          if (fov >= 45.0f)
            fov = 45.0f;
          break;
      }
    }
    if (keys[SDL_SCANCODE_W]) {
      camera.moveForward(delta * cameraSpeed);
    }
    if (keys[SDL_SCANCODE_A]) {
      camera.moveLeft(delta * cameraSpeed);
    }
    if (keys[SDL_SCANCODE_S]) {
      camera.moveBackward(delta * cameraSpeed);
    }
    if (keys[SDL_SCANCODE_D]) {
      camera.moveRight(delta * cameraSpeed);
    }

    if (keys[SDL_SCANCODE_UP]) {
      rotationX += delta * rotationSpeed;
    }
    if (keys[SDL_SCANCODE_DOWN]) {
      rotationX -= delta * rotationSpeed;
    }
    if (keys[SDL_SCANCODE_RIGHT]) {
      rotationY += delta * rotationSpeed;
    }
    if (keys[SDL_SCANCODE_LEFT]) {
      rotationY -= delta * rotationSpeed;
    }

    // Clear and Setup frame
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(VAO);

    // Update matrix
    renderInfo.model =
      glm::rotate(glm::radians(rotationX), glm::vec3(-1, 0, 0)) *
      glm::rotate(glm::radians(rotationY), glm::vec3(0, 0, 1));
    renderInfo.view = camera.makeView();
    renderInfo.projection =
      glm::perspective(glm::radians(fov),
                       float(WINDOW_DEFAULT_W) / WINDOW_DEFAULT_H,
                       0.1f,
                       100.0f);

    // Render objects
    voxel.render(renderInfo);

    // Present
    SDL_GL_SwapWindow(window);

    // "Control" frame rate
    SDL_Delay(1);

    if (auto err = glGetError(); err != GL_NO_ERROR) {
      cerr << gluErrorString(err) << endl;
      return EXIT_FAILURE;
    }

    lastTime    = currentTime;
    currentTime = SDL_GetTicks();
    delta       = (currentTime - lastTime) / 1000;
  }
  return EXIT_SUCCESS;
}
