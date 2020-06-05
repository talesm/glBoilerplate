#pragma once
#include <memory>

// Forward declaration
class ShaderProgram;
class Texture;
class ShaderPool;
class TexturePool;

/**
 * @brief A general Resource Pool containing other resource pools
 *
 */
class ResourcePool
{
public:
  ResourcePool();
  ~ResourcePool();

  /**
   * @brief Get the Shader Program object
   *
   * @param name the shader name. It will try to load if necessary.
   * @return std::shared_ptr<ShaderProgram>
   */
  std::shared_ptr<ShaderProgram> getShaderProgram(const std::string& name);

  /**
   * @brief Get the Texture object
   *
   * @param filePath the path to the texture
   * @return std::shared_ptr<Texture>
   */
  std::shared_ptr<Texture> getTexture(const std::string& filePath);

  // Operational stuff
  ResourcePool(const ResourcePool&) = delete;
  ResourcePool(ResourcePool&&)      = delete;
  ResourcePool& operator=(const ResourcePool&) = delete;
  ResourcePool& operator=(ResourcePool&&) = delete;

private:
  ShaderPool*  mShaderPool;
  TexturePool* mTexturePool;
};
