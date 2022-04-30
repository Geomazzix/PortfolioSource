#include "pch.h"
#include "Core/Entity.h"
#include "Core/EntityRegistry.h"
#include "Core/EngineCore.h"
#include <entt/entt.hpp>

namespace Frac
{
	Entity::Entity(EntityRegistry& registry, const std::string& entityName)
		: m_handle(registry.GetEnTTRegistry().create()), m_entityName(entityName)
	{
		registry.GetEnTTRegistry().emplace<EntityDeletionSinkTag>(m_handle);
		registry.AddEntityToMap(*this);
	}

	Entity::~Entity()
	{
	}

	entt::entity Entity::GetHandle() const
	{
		return m_handle;
	}

	void Entity::SetName(const std::string& newName)
	{
		m_entityName = newName;
	}

	const std::string& Entity::GetEntityName() const
	{
		return m_entityName;
	}

	void Entity::SetParent(Entity& newParentToSet)
	{
		if (m_parent == &newParentToSet)
		{
			FRAC_ASSERT(false, "ASSERT: Set Parent Entity");
		}
		SetParentAndAddChild(newParentToSet, *this);
	}

	Entity& Entity::GetParent()
	{
		FRAC_ASSERT(m_parent != nullptr, "ASSERT: GetParent Nullptr");
		return *m_parent;
	}

	const Entity& Entity::GetParent() const
	{
		FRAC_ASSERT(m_parent != nullptr, "ASSERT: GetParent Nullptr");
		return *m_parent;
	}

	void Entity::RemoveParent()
	{
		m_parent = nullptr;
	}

	void Entity::AddChild(Entity& newChildToAdd)
	{
		if ((std::find(m_children.begin(), m_children.end(), &newChildToAdd) != m_children.end()))
		{
			FRAC_ASSERT(false, "ASSERT: AddChild child already added");
		}
		AddChildAndSetParent(newChildToAdd, *this);
	}

	void Entity::RemoveChild(Entity& childToRemove)
	{
		auto it = std::find(m_children.begin(), m_children.end(), &childToRemove);
		if (it == m_children.end()) FRAC_ASSERT(false, "ASSERT: RemoveChild child already exists");
		m_children.erase(it);
	}

	Frac::Entity& Entity::GetChild(int index)
	{
		FRAC_ASSERT((m_children.size() > index), "ASSERT: GetChild index higher than amount of children");
		return *m_children[index];
	}

	const std::vector<Entity*>& Entity::GetReferenceToChildren()
	{
		return m_children;
	}

	void Entity::AddChildAndSetParent(Entity& newChildToAdd, Entity& newParentToSet)
	{
		m_children.push_back(&newChildToAdd);
		newChildToAdd.m_parent = &newParentToSet;
	}
	void Entity::SetParentAndAddChild(Entity& newParentToSet, Entity& newChildToAdd)
	{
		m_parent = &newParentToSet;
		if (!(std::find(m_children.begin(), m_children.end(), &newChildToAdd) != m_children.end()))
		{
			newParentToSet.m_children.push_back(&newChildToAdd);
		}
	}
}