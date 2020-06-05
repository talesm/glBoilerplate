#pragma once
#include <glm/glm.hpp>

/**
 * @brief A fps-like cameraI
 *
 */
class BasicCamera
{
public:
  /**
   * @brief Construct a new Basic Camera object
   *
   * @param yaw the rotation around y-axis, in degrees. 0 is looking to x-axis,
   * 90 is looking toward z-axis
   * @param pitch the rotation around x-axis. Positive means up, negative down.
   * only values between (-85, +85) are valid
   * @param position th initial camera position, on world coordinates.
   */
  BasicCamera(float yaw, float pitch, glm::vec3 position);
  glm::mat4 makeView() const;

  float yaw() const { return mYaw; }
  float pitch() const { return mPitch; }
  void  rotateTo(float yaw, float pitch)
  {
    mYaw   = yaw;
    mPitch = pitch;
    makeFront();
  }
  void rotateBy(float yaw, float pitch)
  {
    mYaw += yaw;
    mPitch += pitch;
    makeFront();
  }

  const glm::vec3& front() const { return mFront; }
  const glm::vec3& position() const { return mPosition; }
  glm::vec3&       position() { return mPosition; }
  void             position(glm::vec3 position) { mPosition = position; }
  void             moveForward(float delta) { mPosition += delta * mFront; }
  void             moveBackward(float delta) { mPosition -= delta * mFront; }
  void             moveRight(float delta)
  {
    mPosition += glm::normalize(glm::cross(mFront, mUp)) * delta;
  }
  void moveLeft(float delta)
  {
    mPosition -= glm::normalize(glm::cross(mFront, mUp)) * delta;
  }

private:
  void makeFront();

private:
  float     mYaw;
  float     mPitch;
  glm::vec3 mPosition;
  glm::vec3 mFront;
  glm::vec3 mUp;
};
