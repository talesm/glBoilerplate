#pragma once
#include <glm/glm.hpp>
#include "Voxel.hpp"

constexpr int CHUNK_SIDE       = 256;
constexpr int CHUNK_LOAD_DELTA = 32;
constexpr int CHUNK_HALF_SIDE  = CHUNK_SIDE / 2;

class Chunk
{
public:
  __attribute__((always_inline)) Voxel& at(const glm::ivec3& pos)
  {
    return voxels[pos.z & 0xff][pos.y & 0xff][pos.x & 0xff];
  }

  __attribute__((always_inline)) const Voxel& at(const glm::ivec3& pos) const
  {
    return voxels[pos.z & 0xff][pos.y & 0xff][pos.x & 0xff];
  }

private:
  Voxel voxels[CHUNK_SIDE][CHUNK_SIDE][CHUNK_SIDE];
};
