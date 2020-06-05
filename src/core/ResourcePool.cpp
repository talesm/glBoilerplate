#include "ResourcePool.hpp"
#include <unordered_map>
#include "Shader.hpp"
#include "Texture.hpp"

using namespace std;

/**
 * @brief A pool of currently used textures.
 *
 * This allow for easier sharing
 */
class TexturePool
{
public:
  std::shared_ptr<Texture> get(const std::string& fileName);

private:
  std::unordered_map<std::string, std::shared_ptr<Texture>> mTexturePool;
};

/**
 * @brief Allow simplified Shader sharing
 *
 */
class ShaderPool
{
public:
  std::shared_ptr<ShaderProgram> get(const std::string& name);

private:
  std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> mProgramPool;
  std::unordered_map<std::string, std::shared_ptr<Shader>>        mShaderPool;
};

ResourcePool::ResourcePool()
  : mShaderPool(new ShaderPool())
  , mTexturePool(new TexturePool())
{}

ResourcePool::~ResourcePool()
{
  delete mTexturePool;
  delete mShaderPool;
}

std::shared_ptr<ShaderProgram>
ResourcePool::getShaderProgram(const std::string& name)
{
  return mShaderPool->get(name);
}
std::shared_ptr<Texture>
ResourcePool::getTexture(const std::string& filePath)
{
  return mTexturePool->get(filePath);
}

shared_ptr<Texture>
TexturePool::get(const string& fileName)
{
  auto texture = mTexturePool[fileName];
  if (!texture) {
    texture = mTexturePool[fileName] = make_shared<Texture>(fileName.c_str());
  }
  return texture;
}

shared_ptr<ShaderProgram>
ShaderPool::get(const string& name)
{
  shared_ptr<ShaderProgram> program = mProgramPool[name];
  if (!program) {
    string vertexName   = "shaders/" + name + ".vert";
    string fragmentName = "shaders/" + name + ".frag";

    shared_ptr<Shader> vertexShader = mShaderPool[vertexName];
    if (!vertexShader) {
      mShaderPool[vertexName] = vertexShader = make_shared<Shader>(
        Shader::fromFilePath(ShaderType::VERTEX, vertexName.c_str()));
    }

    shared_ptr<Shader> fragmentShader = mShaderPool[fragmentName];
    if (!fragmentShader) {
      mShaderPool[fragmentName] = fragmentShader = make_shared<Shader>(
        Shader::fromFilePath(ShaderType::FRAGMENT, fragmentName.c_str()));
    }
    mProgramPool[name] = program =
      make_shared<ShaderProgram>(*vertexShader, *fragmentShader);
  }
  return program;
}
