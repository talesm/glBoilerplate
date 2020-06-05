#include "Texture.hpp"
#include <stdexcept>
#include <GL/glew.h>
#include "util/stb_image.h"

using namespace std;

Texture::Texture(const char* file)
{
  int            width, height, nrChannels;
  unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);
  if (!data) {
    throw std::runtime_error("Error loading image ");
  }
  glGenTextures(1, &mTextureId);
  glBindTexture(GL_TEXTURE_2D, mTextureId);
  if (nrChannels == 3) {
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 width,
                 height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 data);
  } else if (nrChannels == 4) {
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 width,
                 height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 data);
  }
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(
    GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  stbi_image_free(data);
}

Texture::Texture()
{
  glGenTextures(1, &mTextureId);
  glBindTexture(GL_TEXTURE_2D, mTextureId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture::~Texture()
{
  glDeleteTextures(1, &mTextureId);
}

void
Texture::activate(unsigned textureUnit)
{
  glActiveTexture(textureUnit);
  glBindTexture(GL_TEXTURE_2D, mTextureId);
}

void
Texture::updateTexture(unsigned char* pixels,
                       int            width,
                       int            height,
                       int            channels)
{
  glBindTexture(GL_TEXTURE_2D, mTextureId);
  if (channels == 3) {
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 width,
                 height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 pixels);
  } else if (channels == 4) {
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 width,
                 height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 pixels);
  }
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(
    GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
