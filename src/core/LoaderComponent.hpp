#pragma once
#include <functional>
#include <glm/glm.hpp>
#include "SceneComponent.hpp"

class Chunk;

/**
 * @brief Callback to generate a scene
 *
 * This is called when a chunk needs to be loaded. The given chunk must be
 * filled withe the new content. You must assume the chunk voxels are at
 * indeterminate state.
 *
 * the range is [lowerBound, higherBound, CHUNK_SIDE)),
 * inclusive, exclusive
 */
using SceneLoader = std::function<void(const glm::ivec3& lowerBound,
                                       const glm::ivec3& higherBound,
                                       const glm::ivec3& offset,
                                       Chunk*            chunk)>;

struct LoaderComponent : public SceneComponent
{
  SceneLoader generator;
  glm::ivec3  center{0};

  virtual void onUpdate(float delta) final;

  void reset();

  void sceneGenerator(SceneLoader sceneGenerator)
  {
    generator = sceneGenerator;
  }
};