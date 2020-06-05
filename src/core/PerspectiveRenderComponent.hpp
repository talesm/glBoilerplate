#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "SceneComponent.hpp"

class RenderInfo;
class ResourcePool;
class ShaderProgram;
class VoxelType;
class VoxelTypeImpl;

/**
 * @brief A component to render the scene in perspective
 *
 */
struct PerspectiveRenderComponent : public SceneComponent
{
  ResourcePool* pool;
  float         screenWidth;
  float         screenHeight;
  float         fov       = 30.f;
  float         near      = 2.f;
  float         far       = 50.f;
  float         middleLod = .75f;
  float         farLod    = .95f;
  // VoxelModel                voxelModel;
  mutable unsigned               voxelsRendered = 0;
  std::shared_ptr<ShaderProgram> nearShader;
  std::shared_ptr<ShaderProgram> middleShader;
  std::shared_ptr<ShaderProgram> farShader;
  float                          cosFov;
  int                            axisI;
  std::vector<VoxelTypeImpl>     voxelTypes;
  glm::mat4                      projection;
  glm::mat4                      view;

  PerspectiveRenderComponent(ResourcePool* pool,
                             float         screenWidth,
                             float         screenHeight);
  ~PerspectiveRenderComponent();

  virtual void onUpdate(float delta) final;

  void render() const;

  inline bool renderVoxel(RenderInfo&       renderInfo,
                          const glm::vec3&  cameraPos,
                          const glm::vec3&  cameraDir,
                          const glm::ivec3& iPos) const;

  unsigned insertVoxelType(const VoxelType& type);
};
