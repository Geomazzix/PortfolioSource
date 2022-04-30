#include "pch.h"
#include "SceneManagement/System/TransformSystemController.h"
#include "CoreRenderAPI/Components/Transform.h"
#include <SceneManagement/SceneManager.h>


Frac::TransformSystemController::TransformSystemController():ISystem{eStage::PostUpdate}
{
	EngineCore::GetInstance().GetEnttRegistry().on_update<TOR::Transform>().connect<&TransformSystemController::SetDirtyFlag>(this);
}

void Frac::TransformSystemController::Update(float DeltaTime)
{
	Frac::Entity& root = EngineCore::GetInstance().GetSceneManager().GetActiveScene();
	UpdateTransform(root);
}

void Frac::TransformSystemController::UpdateTransform(Frac::Entity& entity)
{
	Frac::Entity& root = EngineCore::GetInstance().GetSceneManager().GetActiveScene();
	// If the entity is equal to the root we shouldn't take parenting into account but we should still update it cause the root could technically move
	if (!(entity.GetHandle() == root.GetHandle()))
	{
		TOR::Transform& transform = EngineCore::GetInstance().GetFracRegistry().GetComponent<TOR::Transform>(entity);
		if(transform.isDirty)
		{
			
			Frac::Entity parent = entity.GetParent();
			// If the root is the parent don't do anything with the parent transform
			if (parent.GetHandle() == root.GetHandle())
			{
				const glm::mat4 t = glm::translate(glm::mat4{ 1.f }, transform.Position);
				const glm::mat4 r = glm::toMat4(transform.Orientation);
				const glm::mat4 s = glm::scale(glm::mat4{ 1.f }, transform.Scale);
				transform.ModelTransformData = (t * r * s);
			}
			else
			{
				const glm::mat4 t = glm::translate(glm::mat4{ 1.f }, transform.Position);
				const glm::mat4 r = glm::toMat4(transform.Orientation);
				const glm::mat4 s = glm::scale(glm::mat4{ 1.f }, transform.Scale);
				glm::mat4 trs = (t * r * s);
				glm::mat4 p = EngineCore::GetInstance().GetFracRegistry().GetComponent<TOR::Transform>(parent).ModelTransformData;
				glm::mat4 ptrs = p * trs;
				transform.ModelTransformData = ptrs;
			}
			transform.isDirty = false;
		}
	}
	// Update all the childeren recusively
	const std::vector<Frac::Entity*>& childeren = entity.GetReferenceToChildren();
	for (int child = 0; child < childeren.size(); ++child)
	{
		UpdateTransform(entity.GetChild(child));
	}
}

void Frac::TransformSystemController::SetDirtyFlag(entt::registry& registry, entt::entity entity)
{
		registry.get<TOR::Transform>(entity).isDirty = true;
		Frac::Entity& e = Frac::EngineCore::GetInstance().GetFracRegistry().TranslateENTTEntity(entity);
		auto& childeren = e.GetReferenceToChildren();
		for (int i = 0; i < childeren.size(); ++i)
		{
			SetDirtyFlag(registry, e.GetChild(i).GetHandle());
		}
}
