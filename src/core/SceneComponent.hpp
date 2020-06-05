#pragma once
#include <cassert>

struct SceneDetail;

class SceneComponent
{
public:
  SceneComponent()
    : mScene(nullptr)
  {}
  virtual ~SceneComponent() {}

protected:
  SceneDetail* scene() const
  {
    assert(mScene);
    return mScene;
  }

  virtual void onUpdate(float delta) = 0;
  virtual void onAttach(SceneDetail* scene) {}
  virtual void onDetach(SceneDetail* scene) {}

private:
  void attach(SceneDetail* scene)
  {
    assert(!mScene);
    assert(scene);
    mScene = scene;
    onAttach(scene);
  }

  void detach(SceneDetail* scene)
  {
    assert(scene);
    assert(mScene == scene);
    onDetach(scene);
    mScene = nullptr;
  }

private:
  SceneDetail* mScene;
  friend class Scene;
};
