#include "VoxelModel.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include "RenderInfo.hpp"
#include "ResourcePool.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

using namespace std;

struct Vertex
{
  float x, y, z;
  float texR, texS;
};

static Vertex vertices[] = {
  // first  vertex
  {-.5f, -.5f, .5f, 1, 1},
  // second vertex
  {.5f, -.5f, .5f, 0, 1},
  // third vertex
  {-.5f, .5f, .5f, 1, 0},
  // fourth vertex
  {.5f, .5f, .5f, 0, 0}};

string
getShaderType(VoxelDetailType detailType)
{
  switch (detailType) {
    case VoxelDetailType::RELIEF:
      return "relief";
    case VoxelDetailType::BUMP:
      return "bump";
    case VoxelDetailType::GOURAND:
      return "simple";
    default:
      throw runtime_error("Invalid enum");
  }
}

VoxelModel::VoxelModel()
{
  glGenBuffers(1, &mVbo);
  glBindBuffer(GL_ARRAY_BUFFER, mVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

VoxelModel::~VoxelModel()
{
  glDeleteBuffers(1, &mVbo);
}

void
VoxelModel::render(const RenderInfo& renderInfo) const
{
  static glm::vec3 yAxis(0, 1, 0);
  static glm::vec3 xAxis(1, 0, 0);

  auto& shaderProgram = renderInfo.shaderProgram;
  glUseProgram(shaderProgram->shaderProgramId());

  int modelLoc      = shaderProgram->getUniformLocation("model");
  int viewLoc       = shaderProgram->getUniformLocation("view");
  int projectionLoc = shaderProgram->getUniformLocation("projection");
  int tintLoc       = shaderProgram->getUniformLocation("tint");
  int ambient       = shaderProgram->getUniformLocation("ambient");
  int diffuse       = shaderProgram->getUniformLocation("diffuse");
  int specular      = shaderProgram->getUniformLocation("specular");
  int lightColor    = shaderProgram->getUniformLocation("lightColor");
  int lightSource   = shaderProgram->getUniformLocation("lightSource");
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(renderInfo.view));
  glUniformMatrix4fv(
    projectionLoc, 1, GL_FALSE, glm::value_ptr(renderInfo.projection));
  glUniform4fv(tintLoc, 1, glm::value_ptr(renderInfo.tintColor));
  glUniform1f(ambient, renderInfo.lightProperty.ambient);
  glUniform1f(diffuse, renderInfo.lightProperty.diffuse);
  glUniform1f(specular, renderInfo.lightProperty.specular);
  glUniform4fv(lightColor, 1, glm::value_ptr(renderInfo.lightColor));
  glUniform4fv(
    lightSource, 1, glm::value_ptr(glm::normalize(renderInfo.lightSource)));

  glUniform1i(
    glGetUniformLocation(shaderProgram->shaderProgramId(), "inputTex"), 0);
  glUniform1i(
    glGetUniformLocation(shaderProgram->shaderProgramId(), "reliefTex"), 1);

  // Setup textures
  if (renderInfo.surfaceTexture) {
    renderInfo.surfaceTexture->activate(GL_TEXTURE0);
  }
  if (renderInfo.reliefTexture) {
    renderInfo.reliefTexture->activate(GL_TEXTURE1);
  }

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);

  // texture attribute
  glVertexAttribPointer(
    2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texR));
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, mVbo);

  if (renderInfo.faceBitSet & TOP) {
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(renderInfo.model));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  if (renderInfo.faceBitSet & BOTTOM) {
    glUniformMatrix4fv(modelLoc,
                       1,
                       GL_FALSE,
                       glm::value_ptr(renderInfo.model *
                                      glm::rotate(glm::radians(180.f), yAxis)));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  if (renderInfo.faceBitSet & RIGHT) {
    glUniformMatrix4fv(modelLoc,
                       1,
                       GL_FALSE,
                       glm::value_ptr(renderInfo.model *
                                      glm::rotate(glm::radians(90.f), yAxis)));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  if (renderInfo.faceBitSet & LEFT) {
    glUniformMatrix4fv(modelLoc,
                       1,
                       GL_FALSE,
                       glm::value_ptr(renderInfo.model *
                                      glm::rotate(glm::radians(-90.f), yAxis)));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  if (renderInfo.faceBitSet & NEAR) {
    glUniformMatrix4fv(modelLoc,
                       1,
                       GL_FALSE,
                       glm::value_ptr(renderInfo.model *
                                      glm::rotate(glm::radians(90.f), xAxis)));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  if (renderInfo.faceBitSet & FAR) {
    glUniformMatrix4fv(modelLoc,
                       1,
                       GL_FALSE,
                       glm::value_ptr(renderInfo.model *
                                      glm::rotate(glm::radians(-90.f), xAxis)));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
}
