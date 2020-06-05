#include "PerspectiveRenderComponent.hpp"
#include <glm/gtx/transform.hpp>
#include "Camera.hpp"
#include "RenderInfo.hpp"
#include "ResourcePool.hpp"
#include "SceneDetail.hpp"
#include "VoxelModel.hpp"
#include "VoxelType.hpp"
#include "util/fastMath.hpp"

using namespace std;

struct VoxelTypeImpl
{
  shared_ptr<Texture> surfaceTexture;
  shared_ptr<Texture> reliefTexture;
  VoxelTypeImpl(shared_ptr<Texture> surfaceTexture,
                shared_ptr<Texture> reliefTexture)
    : surfaceTexture(move(surfaceTexture))
    , reliefTexture(move(reliefTexture))
  {}
};

PerspectiveRenderComponent::PerspectiveRenderComponent(ResourcePool* pool,
                                                       float screenWidth,
                                                       float screenHeight)
  : pool(pool)
  , screenWidth(screenWidth)
  , screenHeight(screenHeight)
{
  nearShader   = pool->getShaderProgram("relief");
  middleShader = pool->getShaderProgram("bump");
  farShader    = pool->getShaderProgram("simple");
}

PerspectiveRenderComponent::~PerspectiveRenderComponent() = default;

void
PerspectiveRenderComponent::onUpdate(float delta)
{
  float radFov     = glm::radians(fov);
  float ratio      = screenWidth / screenHeight;
  cosFov           = cos(radFov / 2 * ratio + 0.375f);
  projection       = glm::perspective(radFov, ratio, near, far);
  view             = scene()->camera->makeView();
  auto& cameraDir  = scene()->camera->front();
  float cameraMagX = abs(cameraDir.x);
  float cameraMagY = abs(cameraDir.y);
  float cameraMagZ = abs(cameraDir.z);
  if (cameraMagX > cameraMagZ) {
    if (cameraMagX > cameraMagY) {
      axisI = 0;
    } else {
      axisI = 1;
    }
  } else if (cameraMagY > cameraMagZ) {
    axisI = 1;
  } else {
    axisI = 2;
  }
}

void
PerspectiveRenderComponent::render() const
{
  auto& cameraDir = scene()->camera->front();
  auto& cameraPos = scene()->camera->position();
  voxelsRendered  = 0;
  RenderInfo renderInfo;
  renderInfo.projection = projection;
  renderInfo.view       = view;
  int axisJ             = (axisI + 1) % 3;
  int axisK             = (axisI + 2) % 3;
  int begI              = int(cameraPos[axisI]);
  int baseJ             = int(cameraPos[axisJ]);
  int baseK             = int(cameraPos[axisK]);
  int begBiasJ, endBiasJ;
  if (cameraDir[axisJ] > .25) {
    begBiasJ = 1;
    endBiasJ = 3;
  } else if (cameraDir[axisJ] < -.25) {
    begBiasJ = 3;
    endBiasJ = 1;
  } else {
    begBiasJ = 1;
    endBiasJ = 1;
  }

  int begBiasK, endBiasK;
  if (cameraDir[axisK] > .25) {
    begBiasK = 1;
    endBiasK = 3;
  } else if (cameraDir[axisK] < -.25) {
    begBiasK = 3;
    endBiasK = 1;
  } else {
    begBiasK = 1;
    endBiasK = 1;
  }

  if (signbit(cameraDir[axisI])) {
    int        endI = int(cameraPos[axisI] - far);
    glm::ivec3 pos;
    auto       faceBitSet = ~(0x1 << (axisI * 2)) & 0x3f;
    int        baseI      = int(cameraPos[axisI]) + 1;

    switch (axisI) {
      case 0:
        for (pos.x = begI; pos.x >= endI; --pos.x) {
          int ind = baseI - pos.x;

          int begJ = baseJ - ind * begBiasJ;
          int endJ = baseJ + ind * endBiasJ;
          for (pos.y = begJ; pos.y < endJ; ++pos.y) {
            int begK = baseK - ind * begBiasK;
            int endK = baseK + ind * endBiasK;
            for (pos.z = begK; pos.z < endK; ++pos.z) {
              renderInfo.faceBitSet = faceBitSet;
              renderVoxel(renderInfo, cameraPos, cameraDir, pos);
            }
          }
        }
      case 1:
        for (pos.y = begI; pos.y >= endI; --pos.y) {
          int ind = baseI - pos.y;

          int begJ = baseJ - ind * begBiasJ;
          int endJ = baseJ + ind * endBiasJ;
          for (pos.z = begJ; pos.z < endJ; ++pos.z) {
            int begK = baseK - ind * begBiasK;
            int endK = baseK + ind * endBiasK;
            for (pos.x = begK; pos.x < endK; ++pos.x) {
              renderInfo.faceBitSet = faceBitSet;
              renderVoxel(renderInfo, cameraPos, cameraDir, pos);
            }
          }
        }
        break;
      case 2:
        for (pos.z = begI; pos.z >= endI; --pos.z) {
          int ind = baseI - pos.z;

          int begJ = baseJ - ind * begBiasJ;
          int endJ = baseJ + ind * endBiasJ;
          for (pos.x = begJ; pos.x < endJ; ++pos.x) {
            int begK = baseK - ind * begBiasK;
            int endK = baseK + ind * endBiasK;
            for (pos.y = begK; pos.y < endK; ++pos.y) {
              renderInfo.faceBitSet = faceBitSet;
              renderVoxel(renderInfo, cameraPos, cameraDir, pos);
            }
          }
        }
        break;
    }
  } else {
    int        endI = int(cameraPos[axisI] + far);
    glm::ivec3 pos;
    auto       faceBitSet = ~(0x2 << (axisI * 2)) & 0x3f;
    int        baseI      = int(cameraPos[axisI]) - 1;
    switch (axisI) {
      case 0:
        for (pos.x = begI; pos.x < endI; ++pos.x) {
          int ind = pos.x - baseI;

          int begJ = baseJ - ind * begBiasJ;
          int endJ = baseJ + ind * endBiasJ;
          for (pos.y = begJ; pos.y < endJ; ++pos.y) {
            int begK = baseK - ind * begBiasK;
            int endK = baseK + ind * endBiasK;
            for (pos.z = begK; pos.z < endK; ++pos.z) {
              renderInfo.faceBitSet = faceBitSet;
              renderVoxel(renderInfo, cameraPos, cameraDir, pos);
            }
          }
        }
      case 1:
        for (pos.y = begI; pos.y < endI; ++pos.y) {
          int ind = pos.y - baseI;

          int begJ = baseJ - ind * begBiasJ;
          int endJ = baseJ + ind * endBiasJ;
          for (pos.z = begJ; pos.z < endJ; ++pos.z) {
            int begK = baseK - ind * begBiasK;
            int endK = baseK + ind * endBiasK;
            for (pos.x = begK; pos.x < endK; ++pos.x) {
              renderInfo.faceBitSet = faceBitSet;
              renderVoxel(renderInfo, cameraPos, cameraDir, pos);
            }
          }
        }
        break;
      case 2:
        for (pos.z = begI; pos.z < endI; ++pos.z) {
          int ind = pos.z - baseI;

          int begJ = baseJ - ind * begBiasJ;
          int endJ = baseJ + ind * endBiasJ;
          for (pos.x = begJ; pos.x < endJ; ++pos.x) {
            int begK = baseK - ind * begBiasK;
            int endK = baseK + ind * endBiasK;
            for (pos.y = begK; pos.y < endK; ++pos.y) {
              renderInfo.faceBitSet = faceBitSet;
              renderVoxel(renderInfo, cameraPos, cameraDir, pos);
            }
          }
        }
        break;
    }
  }
}

inline bool
PerspectiveRenderComponent::renderVoxel(RenderInfo&       renderInfo,
                                        const glm::vec3&  cameraPos,
                                        const glm::vec3&  cameraDir,
                                        const glm::ivec3& iPos) const
{
  Chunk& chunk    = scene()->chunk;
  auto&  nodeData = chunk.at(iPos);
  if (nodeData.blockType == NO_BLOCK) {
    return false;
  }
  glm::vec3 pos      = glm::vec3(iPos.x + .5f, iPos.y + .5f, iPos.z + .5f);
  glm::vec3 localPos = pos - cameraPos;
  float dist = far * rsqrt(localPos.x * localPos.x + localPos.y * localPos.y +
                           localPos.z * localPos.z);
  if (dist < 1) {
    return false;
  }
  glm::vec3 nPos   = glm::normalize(localPos);
  float     cosPos = glm::dot(cameraDir, nPos);
  if (abs(cosPos) < cosFov) {
    return false;
  }
  if (chunk.at(iPos + glm::ivec3(+1, 0, 0)).blockType != NO_BLOCK) {
    renderInfo.faceBitSet &= ~RIGHT;
  }
  if (chunk.at(iPos + glm::ivec3(-1, 0, 0)).blockType != NO_BLOCK) {
    renderInfo.faceBitSet &= ~LEFT;
  }
  if (chunk.at(iPos + glm::ivec3(0, +1, 0)).blockType != NO_BLOCK) {
    renderInfo.faceBitSet &= ~FAR;
  }
  if (chunk.at(iPos + glm::ivec3(0, -1, 0)).blockType != NO_BLOCK) {
    renderInfo.faceBitSet &= ~NEAR;
  }
  if (chunk.at(iPos + glm::ivec3(0, 0, +1)).blockType != NO_BLOCK) {
    renderInfo.faceBitSet &= ~TOP;
  }
  if (chunk.at(iPos + glm::ivec3(0, 0, -1)).blockType != NO_BLOCK) {
    renderInfo.faceBitSet &= ~BOTTOM;
  }

  if (dist * farLod < 1) {
    renderInfo.shaderProgram = farShader;
  } else if (dist * middleLod < 1) {
    renderInfo.shaderProgram = middleShader;
  } else {
    renderInfo.shaderProgram = nearShader;
  }

  renderInfo.model          = glm::translate(pos);
  auto blockIndex           = nodeData.blockType - 1;
  renderInfo.surfaceTexture = voxelTypes[blockIndex].surfaceTexture;
  renderInfo.reliefTexture  = voxelTypes[blockIndex].reliefTexture;
  static VoxelModel voxelModel;
  voxelModel.render(renderInfo);
  ++voxelsRendered;
  return true;
}

unsigned
PerspectiveRenderComponent::insertVoxelType(const VoxelType& type)
{
  voxelTypes.emplace_back(pool->getTexture(type.surfaceTexture),
                          pool->getTexture(type.reliefTexture));
  return voxelTypes.size();
}
