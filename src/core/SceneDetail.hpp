#pragma once
#include "Chunk.hpp"

class BasicCamera;

/**
 * @brief Contains common scene private elements
 *
 */
struct SceneDetail
{
  BasicCamera* camera;
  Chunk        chunk;

  SceneDetail(BasicCamera* camera)
    : camera(camera)
  {}
};
