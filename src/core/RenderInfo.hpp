#pragma once
#include <memory>
#include <glm/glm.hpp>

enum VoxelFace
{
  LEFT   = 0x1,
  RIGHT  = 0x2,
  NEAR   = 0x4,
  FAR    = 0x8,
  BOTTOM = 0x10,
  TOP    = 0x20
};

struct LightProperty
{
  float ambient{.2f};
  float diffuse{.8f};
  float specular{0.f};
};

// Forward decl
class ShaderProgram;
class Texture;

struct RenderInfo
{
  glm::mat4                      model{1.f};
  glm::mat4                      view{1.f};
  glm::mat4                      projection{1.f};
  glm::vec4                      tintColor{1.f};
  std::shared_ptr<ShaderProgram> shaderProgram;
  std::shared_ptr<Texture>       surfaceTexture;
  std::shared_ptr<Texture>       reliefTexture;
  unsigned                       faceBitSet{0xff};
  LightProperty                  lightProperty;
  glm::vec4                      lightColor{.8f};
  glm::vec4                      lightSource{1.f, 1.f, -1.f, 0.f};
};
