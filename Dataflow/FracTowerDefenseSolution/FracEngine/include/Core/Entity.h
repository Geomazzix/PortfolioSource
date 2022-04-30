#pragma once
#include <entt/entt.hpp>
#include <string>
namespace Frac
{
	struct EntityDeletionSinkTag {};
	class EntityRegistry;
	/// <summary>
	/// Entity wrapper for frac engine to provide simple parenting functions.
	/// </summary>
	class Entity
	{
	public:
		Entity(EntityRegistry& registry, const std::string& entityName);
		~Entity();

		entt::entity GetHandle() const;

		void SetName(const std::string& newName);
		const std::string& GetEntityName() const;

		void SetParent(Entity& newParentToSet);
		void RemoveParent();

		inline bool HasParent() { return m_parent != nullptr; }
		Entity& GetParent();
		const Entity& GetParent() const;

		void AddChild(Entity& newChildToAdd);
		void RemoveChild(Entity& childToRemove);
		Entity& GetChild(int index);

		const std::vector<Entity*>& GetReferenceToChildren();

	private:
		void AddChildAndSetParent(Entity& newChildToAdd, Entity& newParentToSet);
		void SetParentAndAddChild(Entity& newParentToSet, Entity& newChildToAdd);

		entt::entity m_handle;

		std::string m_entityName;

		Entity* m_parent = nullptr;
		std::vector<Entity*> m_children;
		std::vector<Entity*> m_attachments;
	};
}