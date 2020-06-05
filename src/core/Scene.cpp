#include "Scene.hpp"
#include "SceneComponent.hpp"
#include "SceneDetail.hpp"

using namespace std;

Scene::Scene(BasicCamera* camera)
{
  mDetail = make_unique<SceneDetail>(camera);
}

Scene::~Scene() = default;

void
Scene::update(float delta) const
{
  for (auto&& component : mComponents) {
    component->onUpdate(delta);
  }
}

void
Scene::insertComponent(std::shared_ptr<SceneComponent> component)
{
  component->attach(mDetail.get());
  mComponents.emplace_back(move(component));
}

void
Scene::eraseComponent(const std::shared_ptr<SceneComponent>& component)
{
  for (auto it = mComponents.begin(); it != mComponents.end(); ++it) {
    if (component == *it) {
      component->detach(mDetail.get());
      mComponents.erase(it);
      return;
    }
  }
}
