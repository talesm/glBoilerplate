#pragma once
#include <functional>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

class BasicCamera;
class Chunk;
class ResourcePool;
class SceneComponent;
class SceneDetail;
class VoxelType;

/**
 * @brief A rendereable scene
 *
 * It reserves space to at least iLength voxels on each dimension.
 * Each voxel is 0.5 in side length, so the world length is at least iLength*.5
 * in each direction.
 *
 * Voxels are 0-indexed, for easier handling, and so is the world coordinates,
 * which are so always positive too. So the cordinate of any voxel is `c = i / 2
 * + .25`.
 *
 */
class Scene
{
public:
  Scene(BasicCamera* camera);
  ~Scene();

  /**
   * @brief Update all the scene components
   *
   * @param delta
   */
  void update(float delta) const;

  void insertComponent(std::shared_ptr<SceneComponent> component);
  void eraseComponent(const std::shared_ptr<SceneComponent>& component);

private:
  std::unique_ptr<SceneDetail>                 mDetail;
  std::vector<std::shared_ptr<SceneComponent>> mComponents;
};
