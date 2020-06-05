#pragma once

enum class ShaderType : char
{
  FRAGMENT,
  VERTEX,
};

/**
 * @brief Represents a single shader.
 *
 */
class Shader
{
public:
  Shader()
    : mShaderId(0)
  {}
  Shader(ShaderType type, unsigned shaderId)
    : mType(type)
    , mShaderId(shaderId)
  {}
  ~Shader();
  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;
  Shader(Shader&& rhs)
  {
    mType         = rhs.mType;
    mShaderId     = rhs.mShaderId;
    rhs.mShaderId = 0;
  }
  Shader& operator=(Shader&& rhs)
  {
    mType         = rhs.mType;
    mShaderId     = rhs.mShaderId;
    rhs.mShaderId = 0;
    return *this;
  }

  static Shader fromFilePath(ShaderType type, const char* path);

  unsigned shaderId() const { return mShaderId; }

  unsigned release()
  {
    auto shaderId = mShaderId;
    mShaderId     = 0;
    return shaderId;
  }

  ShaderType type() const { return mType; }

private:
  ShaderType mType;
  unsigned   mShaderId;
};

/**
 * @brief  linked shader program
 *
 */
class ShaderProgram
{
public:
  template<class... SHADER>
  ShaderProgram(const Shader& shader1, const SHADER&... shaders)
    : ShaderProgram()
  {
    insertShaders(shader1, shaders...);
    link();
  }
  ~ShaderProgram();
  ShaderProgram(const ShaderProgram&) = delete;
  ShaderProgram& operator=(const ShaderProgram&) = delete;
  ShaderProgram(ShaderProgram&& rhs)
  {
    mShaderProgramId     = rhs.mShaderProgramId;
    rhs.mShaderProgramId = 0;
  }
  ShaderProgram& operator=(ShaderProgram&& rhs)
  {
    mShaderProgramId     = rhs.mShaderProgramId;
    rhs.mShaderProgramId = 0;
    return *this;
  }

  unsigned shaderProgramId() const { return mShaderProgramId; }
  int getUniformLocation(const char *locName) const;

private:
  ShaderProgram();
  void insertShader(const Shader& shader);
  void link();

  template<class... SHADER>
  void insertShaders(const Shader& shader1, const SHADER&... shaders)
  {
    insertShader(shader1);
    insertShaders(shaders...);
  }
  void insertShaders() {}

//atributes
  unsigned mShaderProgramId;
};
