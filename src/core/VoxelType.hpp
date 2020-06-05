#pragma once
#include <string>

struct VoxelType
{
  std::string surfaceTexture;
  std::string reliefTexture;

  VoxelType& withSurface(std::string value)
  {
    surfaceTexture = std::move(value);
    return *this;
  }

  VoxelType& withRelief(std::string value)
  {
    reliefTexture = std::move(value);
    return *this;
  }
};