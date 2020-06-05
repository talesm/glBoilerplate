#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

BasicCamera::BasicCamera(float yaw, float pitch, glm::vec3 position)
  : mYaw(yaw)
  , mPitch(pitch)
  , mPosition(position)
  , mUp(0.f, 0.f, 1.f)
{
  makeFront();
}

void
BasicCamera::makeFront()
{
  if (mPitch > 89.0f) {
    mPitch = 89.0f;
  } else if (mPitch < -89.0f) {
    mPitch = -89.0f;
  }
  if (mYaw >= 360.f) {
    mPitch -= 360.f;
  } else if (mYaw <= -360.f) {
    mPitch += 360.f;
  }
  glm::vec3 front;
  front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
  front.y = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
  front.z = sin(glm::radians(mPitch));
  mFront  = glm::normalize(front);
}

glm::mat4
BasicCamera::makeView() const
{
  return glm::lookAt(mPosition, mPosition + mFront, mUp);
}
