#include <cstdlib>
#include <iostream>
#include <sstream>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include "GuiController.hpp"
#include "core/Camera.hpp"
#include "core/Chunk.hpp"
#include "core/LoaderComponent.hpp"
#include "core/PerspectiveRenderComponent.hpp"
#include "core/ResourcePool.hpp"
#include "core/Scene.hpp"
#include "core/VoxelType.hpp"

constexpr int WINDOW_DEFAULT_W = 1200;
constexpr int WINDOW_DEFAULT_H = 796;

using namespace std;

enum class Shape : int
{
  UNDEFINED = -1,
  PLANE_XY,
  SOLID_CUBE,
  WIRE_CUBE,
  SPHERE
};

enum class ShapeSize : int
{
  UNDEFINED = -1,
  SIZE_10,
  SIZE_100,
  SIZE_1000,
  INFINITE
};

void
makeSceneShape(const shared_ptr<LoaderComponent>& loader,
               Shape                              shape,
               ShapeSize                          size,
               unsigned                           baseVoxel);

int
main(int argc, char const* argv[])
{
  string surfaceTexture = "res/tile1.jpg";
  string reliefTexture  = "res/tile1.png";

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
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

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

  SDL_GLContext glContext = SDL_GL_CreateContext(window);
  GLenum        err       = glewInit();
  if (GLEW_OK != err) {
    /* Problem: glewInit failed, something is seriously wrong. */
    cerr << "Error: " << glewGetErrorString(err) << endl;
    return 1;
  }
  cout << "Status: Using GLEW" << glewGetString(GLEW_VERSION) << endl;

  // Use Vsync
  bool vSync = true;
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

  glm::vec3     initalCameraPos(-4.5f, -4.5f, 3.25f);
  float         angleH = 45;
  float         angleV = 0;
  BasicCamera   camera(angleH, angleV, initalCameraPos);
  ResourcePool  resourcePool;
  GuiController controller(window, glContext, &resourcePool);
  Scene         scene(&camera);
  auto          loaderComponent = make_shared<LoaderComponent>();
  auto          renderComponent = make_shared<PerspectiveRenderComponent>(
    &resourcePool, WINDOW_DEFAULT_W, WINDOW_DEFAULT_H);
  scene.insertComponent(loaderComponent);
  scene.insertComponent(renderComponent);

  Shape     shape     = Shape::PLANE_XY;
  ShapeSize shapeSize = ShapeSize::INFINITE;
  auto      baseVoxel = renderComponent->insertVoxelType(
    VoxelType{}.withSurface(surfaceTexture).withRelief(reliefTexture));
  float clearColor[4] = {.25f, .65f, .999f, 1.f};

  bool mouseGrab = false;

  float  frameRate        = 0;
  float& fov              = renderComponent->fov;
  float& near             = renderComponent->near;
  float& far              = renderComponent->far;
  float  cameraSpeed      = 10.f; // adjust accordingly
  float  mouseSensitivity = 0.05f;
  float  lodFarPercent    = renderComponent->farLod * 100;
  float  lodMiddlePercent = renderComponent->middleLod * 100;

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);

  auto  keys        = SDL_GetKeyboardState(nullptr);
  float currentTime = SDL_GetTicks(), lastTime = currentTime, delta = 0;
  float frameCount = 0;
  float timeCount  = 0;
  bool  showDemo   = false;
  for (;;) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      if (!mouseGrab && controller.processEvent(ev)) {
        continue;
      }
      switch (ev.type) {
        case SDL_QUIT:
          return EXIT_SUCCESS;
        case SDL_KEYDOWN:
          switch (ev.key.keysym.scancode) {
            case SDL_SCANCODE_F2:
              camera.rotateTo(0, 0);
              camera.position(initalCameraPos);
              break;
            case SDL_SCANCODE_M:
              mouseGrab = !mouseGrab;
              SDL_SetRelativeMouseMode(mouseGrab ? SDL_TRUE : SDL_FALSE);
            default:
              break;
          }
          break;
        case SDL_MOUSEMOTION:
          if (mouseGrab || (ev.motion.state & SDL_BUTTON_LMASK)) {
            angleH -= ev.motion.xrel * mouseSensitivity;
            angleV -= ev.motion.yrel * mouseSensitivity;
            if (angleH >= 360) {
              angleH -= 360.f;
            } else if (angleH < 0) {
              angleH += 360.f;
            }
          }
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
    if (!ImGui::GetIO().WantCaptureKeyboard) {
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
    }
    // Update
    camera.rotateTo(angleH, angleV);
    renderComponent->middleLod = lodMiddlePercent / 100.f;
    renderComponent->farLod    = lodFarPercent / 100.f;
    makeSceneShape(loaderComponent, shape, shapeSize, baseVoxel);
    scene.update(delta);

    // Clear and Setup frame
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(VAO);

    // Present
    renderComponent->render();
    controller.newFrame();
    {
      ImGui::Begin("Tweaks");
      ImGui::Text("FPS %.2f", frameRate);
      ImGui::Text("Voxels Rendered %d", renderComponent->voxelsRendered);
      ImGui::Combo("Shape",
                   reinterpret_cast<int*>(&shape),
                   "PLANE XY\0SOLID CUBE\0WIRE CUBE\0SPHERE\0");

      ImGui::Combo("Shape Size",
                   reinterpret_cast<int*>(&shapeSize),
                   "   10\0  100\0 1000\0INFIN\0");
      ImGui::ColorEdit3("Clear Color", clearColor);
      ImGui::Checkbox("VSync", &vSync);
      if (vSync) {
        if (SDL_GL_GetSwapInterval() == 0) {
          if (SDL_GL_SetSwapInterval(1) < 0) {
            cerr << SDL_GetError() << endl;
          }
        }
      } else {
        if (SDL_GL_GetSwapInterval() == 1) {
          if (SDL_GL_SetSwapInterval(0) < 0) {
            cerr << SDL_GetError() << endl;
          }
        }
      }
      ImGui::Separator();

      if (ImGui::CollapsingHeader("Projection")) {
        ImGui::LabelText("Projection Type", "Perspective");
        ImGui::SliderFloat("near", &near, 0.125f, 5.f);
        ImGui::SliderFloat("far", &far, 50.f, 500.f);
        ImGui::SliderFloat("FOV", &fov, 1.f, 45.f);
      }

      if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat3("Cam Pos", &camera.position().x);
        ImGui::SliderFloat("Horizontal Angle", &angleH, 0, 360);
        ImGui::SliderFloat("Vertical Angle", &angleV, -89, +89);
      }

      if (ImGui::CollapsingHeader("LOD")) {
        ImGui::SliderFloat("Far Limit", &lodFarPercent, 0, 100, "%.2f%%");
        ImGui::SliderFloat(
          "Middle Limit", &lodMiddlePercent, 0, lodFarPercent, "%.2f%%");
      }

      ImGui::Separator();
      ImGui::Checkbox("Show Demo", &showDemo);
      ImGui::End();
      if (showDemo) {
        ImGui::ShowDemoWindow(&showDemo);
      }
    }
    controller.render();

    SDL_GL_SwapWindow(window);
    // "Control" frame rate
    SDL_Delay(1);

    if (auto err = glGetError(); err != GL_NO_ERROR) {
      cerr << gluErrorString(err) << endl;
      return EXIT_FAILURE;
    }

    lastTime      = currentTime;
    currentTime   = SDL_GetTicks();
    float deltaMs = currentTime - lastTime;
    delta         = deltaMs / 1000;
    timeCount += deltaMs;
    frameCount += 1;
    if (frameCount >= 10) {
      frameRate = frameCount / timeCount * 1000;
      timeCount = frameCount = 0;
    }
  }
  return EXIT_SUCCESS;
}

void
makeSceneShape(const shared_ptr<LoaderComponent>& loader,
               Shape                              shape,
               ShapeSize                          size,
               unsigned                           baseVoxel)
{
  static Shape     oldShape = Shape::UNDEFINED;
  static ShapeSize oldSize  = ShapeSize::UNDEFINED;
  if (oldShape == shape && oldSize == size) {
    return;
  }
  oldShape         = shape;
  oldSize          = size;
  int sizeInVoxels = pow(10, static_cast<int>(size) + 1);
  switch (shape) {
    case Shape::PLANE_XY:
      if (size == ShapeSize::INFINITE) {
        loader->sceneGenerator([baseVoxel](const glm::ivec3& lowerBound,
                                           const glm::ivec3& higherBound,
                                           const glm::ivec3& offset,
                                           Chunk*            chunk) {
          for (int i = lowerBound.z; i < higherBound.z; ++i) {
            for (int j = lowerBound.y; j < higherBound.y; ++j) {
              for (int k = lowerBound.x; k < higherBound.x; ++k) {
                auto pos = glm::ivec3(k, j, i);
                if (pos.z == -offset.z) {
                  chunk->at(pos).blockType = baseVoxel;
                } else {
                  chunk->at(pos).blockType = NO_BLOCK;
                }
              }
            }
          }
        });
      } else {
        loader->sceneGenerator(
          [baseVoxel, sizeInVoxels](const glm::ivec3& lowerBound,
                                    const glm::ivec3& higherBound,
                                    const glm::ivec3& offset,
                                    Chunk*            chunk) {
            for (int i = lowerBound.z; i < higherBound.z; ++i) {
              for (int j = lowerBound.y; j < higherBound.y; ++j) {
                for (int k = lowerBound.x; k < higherBound.x; ++k) {
                  auto pos = glm::ivec3(k, j, i);
                  if ((pos.x + offset.x < sizeInVoxels && pos.x >= offset.x) &&
                      (pos.y + offset.y < sizeInVoxels && pos.y >= offset.y) &&
                      pos.z == -offset.z) {
                    chunk->at(pos).blockType = baseVoxel;
                  } else {
                    chunk->at(pos).blockType = NO_BLOCK;
                  }
                }
              }
            }
          });
      }
      break;
    case Shape::SOLID_CUBE:
      loader->sceneGenerator(
        [baseVoxel, sizeInVoxels](const glm::ivec3& lowerBound,
                                  const glm::ivec3& higherBound,
                                  const glm::ivec3& offset,
                                  Chunk*            chunk) {
          for (int i = lowerBound.z; i < higherBound.z; ++i) {
            for (int j = lowerBound.y; j < higherBound.y; ++j) {
              for (int k = lowerBound.x; k < higherBound.x; ++k) {
                auto pos = glm::ivec3(k, j, i);

                if ((pos.x + offset.x < sizeInVoxels && pos.x >= -offset.x) &&
                    (pos.y + offset.y < sizeInVoxels && pos.y >= -offset.y) &&
                    (pos.z + offset.z < sizeInVoxels && pos.z >= -offset.z)) {
                  chunk->at(pos).blockType = baseVoxel;
                } else {
                  chunk->at(pos).blockType = NO_BLOCK;
                }
              }
            }
          }
        });
      break;
    case Shape::WIRE_CUBE:
      loader->sceneGenerator([baseVoxel,
                              sizeInVoxels](const glm::ivec3& lowerBound,
                                            const glm::ivec3& higherBound,
                                            const glm::ivec3& offset,
                                            Chunk*            chunk) {
        for (int i = lowerBound.z; i < higherBound.z; ++i) {
          for (int j = lowerBound.y; j < higherBound.y; ++j) {
            for (int k = lowerBound.x; k < higherBound.x; ++k) {
              auto pos    = glm::ivec3(k, j, i);
              auto effPos = pos + offset;
              if (effPos.x <= sizeInVoxels && effPos.y <= sizeInVoxels &&
                  (effPos.z == 0 || effPos.z == sizeInVoxels)) {
                chunk->at(pos).blockType = baseVoxel;
              } else if (effPos.x <= sizeInVoxels &&
                         (effPos.y == 0 || effPos.y == sizeInVoxels) &&
                         effPos.z <= sizeInVoxels) {
                chunk->at(pos).blockType = baseVoxel;
              } else if ((effPos.x == 0 || effPos.x == sizeInVoxels) &&
                         effPos.y <= sizeInVoxels && effPos.z <= sizeInVoxels) {
                chunk->at(pos).blockType = baseVoxel;
              } else {
                chunk->at(pos).blockType = NO_BLOCK;
              }
            }
          }
        }
      });
      break;
    case Shape::SPHERE:
      loader->sceneGenerator(
        [baseVoxel, sizeInVoxels](const glm::ivec3& lowerBound,
                                  const glm::ivec3& higherBound,
                                  const glm::ivec3& offset,
                                  Chunk*            chunk) {
          float radius = sizeInVoxels / 2.f;
          for (int i = lowerBound.z; i < higherBound.z; ++i) {
            for (int j = lowerBound.y; j < higherBound.y; ++j) {
              for (int k = lowerBound.x; k < higherBound.x; ++k) {
                auto pos = glm::ivec3(k, j, i);
                if (sqrtf(powf(pos.x + .5f + offset.x, 2) +
                          powf(pos.y + .5f + offset.y, 2) +
                          powf(pos.z + .5f + offset.z, 2)) <= radius) {
                  chunk->at(pos).blockType = baseVoxel;
                } else {
                  chunk->at(pos).blockType = NO_BLOCK;
                }
              }
            }
          }
        });
      break;
    default:
      throw std::runtime_error("Unimplemented");
  }
  loader->reset();
}
