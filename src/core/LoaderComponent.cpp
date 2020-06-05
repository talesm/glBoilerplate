#include "LoaderComponent.hpp"
#include "Camera.hpp"
#include "SceneDetail.hpp"
#include "util/fastMath.hpp"

using namespace std;

inline int
getAreaToReload(int offset)
{
  int areaToReload = abs(offset) - (CHUNK_HALF_SIDE - CHUNK_LOAD_DELTA);
  int remaining    = areaToReload % CHUNK_LOAD_DELTA;
  if (remaining) {
    return areaToReload + CHUNK_LOAD_DELTA - remaining;
  }
  return areaToReload;
}

template<int axisI, int axisJ = (axisI + 1) % 3, int axisK = (axisI + 2) % 3>
void
checkAndRefreshChunk(Chunk*            chunk,
                     SceneLoader&      generator,
                     glm::ivec3&       center,
                     const glm::ivec3& delta)
{
  if (delta[axisI] < -(CHUNK_HALF_SIDE - CHUNK_LOAD_DELTA)) {
    int areaToReload = getAreaToReload(delta[axisI]);
    center[axisI] -= areaToReload;
    int        r = floorMod(center[axisJ] - CHUNK_HALF_SIDE, CHUNK_SIDE);
    int        q = floorMod(center[axisK] - CHUNK_HALF_SIDE, CHUNK_SIDE);
    glm::ivec3 lBound, hBound, offset;
    lBound[axisI] = CHUNK_SIDE - areaToReload;
    lBound[axisJ] = r;
    lBound[axisK] = q;
    hBound[axisI] = CHUNK_SIDE;
    hBound[axisJ] = CHUNK_SIDE;
    hBound[axisK] = CHUNK_SIDE;
    offset[axisI] = center[axisI] - CHUNK_SIDE * 3 / 2 + areaToReload;
    offset[axisJ] = center[axisJ] - CHUNK_HALF_SIDE - r;
    offset[axisK] = center[axisK] - CHUNK_HALF_SIDE - q;
    generator(lBound, hBound, offset, chunk);
    if (r) {
      lBound[axisJ] = 0;
      lBound[axisK] = q;
      hBound[axisJ] = r;
      hBound[axisK] = CHUNK_SIDE;
      offset[axisJ] = center[axisJ] + CHUNK_HALF_SIDE - r;
      offset[axisK] = center[axisK] - CHUNK_HALF_SIDE - q;
      generator(lBound, hBound, offset, chunk);
    }
    if (q) {
      lBound[axisJ] = r;
      lBound[axisK] = 0;
      hBound[axisJ] = CHUNK_SIDE;
      hBound[axisK] = q;
      offset[axisJ] = center[axisJ] - CHUNK_HALF_SIDE - r;
      offset[axisK] = center[axisK] + CHUNK_HALF_SIDE - q;
      generator(lBound, hBound, offset, chunk);
      if (r) {
        lBound[axisJ] = 0;
        lBound[axisK] = 0;
        hBound[axisJ] = r;
        hBound[axisK] = q;
        offset[axisJ] = center[axisJ] + CHUNK_HALF_SIDE - r;
        offset[axisK] = center[axisK] + CHUNK_HALF_SIDE - q;
        generator(lBound, hBound, offset, chunk);
      }
    }
  } else if (delta[axisI] >= (CHUNK_HALF_SIDE - CHUNK_LOAD_DELTA)) {
    int areaToReload = getAreaToReload(delta[axisI]);
    center[axisI] += areaToReload;
    int        r = floorMod(center[axisJ] - CHUNK_HALF_SIDE, CHUNK_SIDE);
    int        q = floorMod(center[axisK] - CHUNK_HALF_SIDE, CHUNK_SIDE);
    glm::ivec3 lBound, hBound, offset;
    lBound[axisI] = 0;
    lBound[axisJ] = r;
    lBound[axisK] = q;
    hBound[axisI] = areaToReload;
    hBound[axisJ] = CHUNK_SIDE;
    hBound[axisK] = CHUNK_SIDE;
    offset[axisI] = center[axisI] - CHUNK_HALF_SIDE + areaToReload;
    offset[axisJ] = center[axisJ] - CHUNK_HALF_SIDE - r;
    offset[axisK] = center[axisK] - CHUNK_HALF_SIDE - q;
    generator(lBound, hBound, offset, chunk);
    if (r) {
      lBound[axisJ] = 0;
      lBound[axisK] = q;
      hBound[axisJ] = r;
      hBound[axisK] = CHUNK_SIDE;
      offset[axisJ] = center[axisJ] + CHUNK_HALF_SIDE - r;
      offset[axisK] = center[axisK] - CHUNK_HALF_SIDE - q;
      generator(lBound, hBound, offset, chunk);
    }
    if (q) {
      lBound[axisJ] = r;
      lBound[axisK] = 0;
      hBound[axisJ] = CHUNK_SIDE;
      hBound[axisK] = q;
      offset[axisJ] = center[axisJ] - CHUNK_HALF_SIDE - r;
      offset[axisK] = center[axisK] + CHUNK_HALF_SIDE - q;
      generator(lBound, hBound, offset, chunk);
      if (r) {
        lBound[axisJ] = 0;
        lBound[axisK] = 0;
        hBound[axisJ] = r;
        hBound[axisK] = q;
        offset[axisJ] = center[axisJ] + CHUNK_HALF_SIDE - r;
        offset[axisK] = center[axisK] + CHUNK_HALF_SIDE - q;
        generator(lBound, hBound, offset, chunk);
      }
    }
  }
}

void
LoaderComponent::onUpdate(float delta)
{
  if (!generator)
    return;
  glm::ivec3 offset = glm::ivec3(scene()->camera->position()) - center;
  auto&      chunk  = scene()->chunk;
  checkAndRefreshChunk<0>(&chunk, generator, center, offset);
  checkAndRefreshChunk<1>(&chunk, generator, center, offset);
  checkAndRefreshChunk<2>(&chunk, generator, center, offset);
}

void
LoaderComponent::reset()
{
  if (!generator) {
    return;
  }
  center = glm::ivec3(CHUNK_SIDE / 2);
  generator(
    glm::ivec3(0), glm::vec3(CHUNK_SIDE), glm::vec3(0), &scene()->chunk);
}
