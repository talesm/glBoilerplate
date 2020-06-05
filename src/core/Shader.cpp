#include "Shader.hpp"
#include <stdexcept>
#include <GL/glew.h>
#include "util/fileReader.hpp"

using namespace std;

Shader::~Shader()
{
  if (mShaderId) {
    glDeleteShader(mShaderId);
  }
}

Shader
Shader::fromFilePath(ShaderType type, const char* path)
{
  unsigned shaderId;
  switch (type) {
    case ShaderType::FRAGMENT:
      shaderId = glCreateShader(GL_FRAGMENT_SHADER);
      break;
    case ShaderType::VERTEX:
      shaderId = glCreateShader(GL_VERTEX_SHADER);
      break;
    default:
      throw std::logic_error("Invalid shader type");
  }
  auto shaderSource     = readFile(path);
  auto shaderSourceChar = shaderSource.c_str();
  glShaderSource(shaderId, 1, &shaderSourceChar, NULL);
  glCompileShader(shaderId);

  int success;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
    throw std::runtime_error(infoLog);
  }

  return Shader(type, shaderId);
}

// Programs

ShaderProgram::~ShaderProgram()
{
  if (mShaderProgramId) {
    glDeleteProgram(mShaderProgramId);
  }
}

ShaderProgram::ShaderProgram()
{
  mShaderProgramId = glCreateProgram();
}

int
ShaderProgram::getUniformLocation(const char* locName) const
{
  return glGetUniformLocation(mShaderProgramId, locName);
}

void
ShaderProgram::insertShader(const Shader& shader)
{
  glAttachShader(mShaderProgramId, shader.shaderId());
}
void
ShaderProgram::link()
{
  glLinkProgram(mShaderProgramId);
  int success;
  glGetProgramiv(mShaderProgramId, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(mShaderProgramId, 512, NULL, infoLog);
    throw std::runtime_error(infoLog);
  }
}
