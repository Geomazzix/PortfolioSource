#pragma once

#include "Core/Entity.h"
#include "Utility/FracAssert.h"
#include <entt/entt.hpp>
namespace Frac
{
	class EntityRegistry
	{
	public:
		EntityRegistry();
		~EntityRegistry() = default;

		template<typename T>
		T& GetComponent(const Entity& entityToGetComponentFrom);

		template<typename T>
		const T& GetComponent(const Entity& entityToGetComponentFrom) const;

		template<typename T>
		bool HasComponent(const Entity& entityToGetComponentFrom);

		template<typename T>
		const Entity& RemoveComponent(const Entity& entityToGetComponentFrom);

		template<typename T, typename... ComponentArgs>
		const Entity& AddComponent(const Entity& entityToGetComponentFrom, ComponentArgs&&... args);

		entt::registry& GetEnTTRegistry() { return m_registry; }

		const entt::registry& GetEnTTRegistry() const { return m_registry; }
		Frac::Entity& TranslateENTTEntity(const entt::entity& entity);
	protected:
		// The reason for the friend class is quite simple in this case we want the Entity to be able to add itself to the registry when created.
		// We don't want any other object to call this
		friend class Entity;
		void AddEntityToMap(Frac::Entity& entity);
		void RemoveEntityFromMap(entt::registry& registry, entt::entity entity);
	private:
		entt::registry m_registry;
		std::unordered_map<entt::entity, Frac::Entity*> m_entityMap;
	};

	inline EntityRegistry::EntityRegistry()
	{
		m_registry.on_destroy<EntityDeletionSinkTag>().connect<&EntityRegistry::RemoveEntityFromMap>(this);
	}

	inline Frac::Entity& EntityRegistry::TranslateENTTEntity(const entt::entity& entity)
	{
		return *m_entityMap[entity];
	}

	inline void EntityRegistry::AddEntityToMap(Frac::Entity& entity)
	{
		m_entityMap.insert({ entity.GetHandle(),&entity });
	}

	inline void EntityRegistry::RemoveEntityFromMap(entt::registry& registry, entt::entity entity)
	{
		Frac::Entity& e = TranslateENTTEntity(entity);
		auto& childeren = e.GetReferenceToChildren();
		if (!childeren.empty())
		{
			do
			{
				m_registry.destroy(childeren.front()->GetHandle());
			} while (!childeren.empty());
		}
		if(m_entityMap[entity]->HasParent())
		m_entityMap[entity]->GetParent().RemoveChild(*m_entityMap[entity]);
		m_entityMap.erase(entity);
		delete& e;
	}

	template<typename T>
	const Entity& Frac::EntityRegistry::RemoveComponent(const Entity& entityToGetComponentFrom)
	{
		FRAC_ASSERT(HasComponent<T>(entityToGetComponentFrom), "ASSERT: Entity does not exist");
		m_registry.remove<T>(entityToGetComponentFrom.GetHandle());
		return entityToGetComponentFrom;
	}

	template<typename T, typename... ComponentArgs>
	const Entity& Frac::EntityRegistry::AddComponent(const Entity& entity, ComponentArgs&&... args)
	{
		FRAC_ASSERT(!HasComponent<T>(entity), "ASSERT: Component already existing");
		m_registry.emplace<T>(entity.GetHandle(), std::forward<ComponentArgs>(args)...);
		return entity;
	}

	template<typename T>
	bool Frac::EntityRegistry::HasComponent(const Entity& entityToGetComponentFrom)
	{
		return 	m_registry.any_of<T>(entityToGetComponentFrom.GetHandle());
	}

	template<typename T>
	const T& Frac::EntityRegistry::GetComponent(const Entity& entityToGetComponentFrom) const
	{
		FRAC_ASSERT(HasComponent<T>(entityToGetComponentFrom), "ASSERT:GetComponent Component not on entity");
		return m_registry.get<T>(entityToGetComponentFrom.GetHandle());
	}

	template<typename T>
	T& Frac::EntityRegistry::GetComponent(const Entity& entityToGetComponentFrom)
	{
		FRAC_ASSERT(HasComponent<T>(entityToGetComponentFrom), "ASSERT:GetComponent Component not on entity");
		return m_registry.get<T>(entityToGetComponentFrom.GetHandle());
	}
}