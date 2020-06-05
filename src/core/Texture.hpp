#pragma once

/**
 * @brief Represents a texture
 *
 */
class Texture
{
public:
  Texture(const char* file);
  Texture();
  ~Texture();
  Texture(const Texture&) = delete;
  Texture(Texture&&)      = default;
  Texture& operator=(const Texture&) = delete;
  Texture& operator=(Texture&&) = default;

  void     activate(unsigned textureUnit);
  unsigned textureId() const { return mTextureId; }
  void updateTexture(unsigned char* pixels, int width, int height, int channels = 4);

private:
  unsigned mTextureId;
};
