#pragma once

// Forward declarations
class RenderInfo;

enum class VoxelDetailType
{
  RELIEF,
  BUMP,
  GOURAND
};

/**
 * @brief A voxel model
 *
 */
class VoxelModel
{
public:
  VoxelModel();
  ~VoxelModel();
  VoxelModel(const VoxelModel&) = delete;
  VoxelModel(VoxelModel&&)      = delete;
  VoxelModel& operator=(const VoxelModel&) = delete;
  VoxelModel& operator=(VoxelModel&&) = delete;

  /**
   * @brief Render the face
   *
   */
  void render(const RenderInfo& renderInfo) const;

private:
  unsigned int mVbo;
};
