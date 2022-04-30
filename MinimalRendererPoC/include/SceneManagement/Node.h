#pragma once

#include "ENodeType.h"
#include <vector>
#include <memory>
#include <string>

#include "glm/gtx/quaternion.hpp"
#include "glm/glm.hpp"

namespace mrpoc
{
	/// <summary>
	/// The node class represents one object in the scene graph. 
	/// It keeps track of a parent/children relationship as well.
	/// </summary>
	class Node
	{
	public:
		Node();
		~Node() = default;

		void InitializeNode(ENodeType nodeType, std::shared_ptr<Node> aParent);

		void SetLocalTransform(const glm::mat4x4& aTransformValue);
		const glm::mat4x4& GetLocalTransform();
		const glm::mat4x4& GetWorldTransform();

		void Translate(const glm::vec3& aTranslationVector);
		void Rotate(const glm::quat& rotationToApply);
		void Rotate(const glm::vec3& eulerAngles);
		void Scale(const glm::vec3& newScaleToApply);

		void Update();

		glm::vec3 GetTranslation();
		glm::vec3 GetScale();
		glm::quat GetRotation();

		std::weak_ptr<Node> GetParent();

		std::vector<std::shared_ptr<Node>>& GetChildren();
		void AddChild(std::shared_ptr<Node> aNewChild);

		ENodeType GetNodeType() const;
		int GetDataIndex() const;

		void LookAt(glm::vec3 a_targetPos);
		void SetDataIndex(int valueToSet) { m_dataIndex = valueToSet; }
		std::string GetName() const { return m_name; }
		void SetName(std::string valueToSet) { m_name = valueToSet; }
	
		Node* GetActiveCamera(int cameraNode);
		void SearchTree(int nodeIndex, Node*& found);

	private:
		void SetTransformDirtyFlag(bool value = true);

		bool m_initialized = false;
		bool m_transformDirty = false;

		ENodeType m_typeOfNode;

		int m_dataIndex;

		std::string m_name;

		std::weak_ptr<Node> m_parent;
		std::vector<std::shared_ptr<Node>> m_children;

		glm::vec3 m_translation = glm::vec3(0);
		glm::vec3 m_scale = glm::vec3(1);
		glm::quat m_rotation = glm::quat(1.0, 0.0, 0.0, 0.0);

		glm::mat4x4 m_localTransform = glm::mat4x4(1);
		glm::mat4x4 m_worldTransform = glm::mat4x4(1);

		void UpdateLocalTransformMatrix();
	};
}