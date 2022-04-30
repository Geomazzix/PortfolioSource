#include "MrpocPch.h"
#include "SceneManagement/Node.h"
#include <spdlog/spdlog.h>

#include "glm/gtx/transform.hpp"
#include <glm/gtx/matrix_decompose.hpp>

namespace mrpoc
{
	Node::Node()
	{
		m_transformDirty = true;
		m_localTransform = glm::mat4x4(1);
		m_worldTransform = glm::mat4x4(1);
		m_translation = glm::vec3(0,0,0);
		m_rotation = glm::quat(1,0,0,0);
		m_scale = glm::vec3(1,1,1);
		m_dataIndex = -1;
		m_typeOfNode = ENodeType::EMPTY;
	}

	void Node::InitializeNode(ENodeType nodeType, std::shared_ptr<Node> aParent)
	{
		m_typeOfNode = nodeType;
	
		if (nodeType == ENodeType::ROOT)
		{
			m_initialized = true;
		}
		else
		{
			if(aParent)
			{
				m_parent = aParent;
				m_initialized = true;
			}
			else
			{
				spdlog::error("Invalid Node parent");
			}
		}
	}

	void Node::SetLocalTransform(const glm::mat4x4& aTransformValue)
	{
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(aTransformValue,m_scale,m_rotation,m_translation, skew,perspective);
		m_transformDirty = true;
	}

	const glm::mat4x4& Node::GetLocalTransform()
	{
		return m_localTransform;
	}

	void Node::SetTransformDirtyFlag(bool value /*= true*/)
	{
		m_transformDirty = value;
	}

	void Node::UpdateLocalTransformMatrix()
	{
		m_localTransform = glm::mat4(1.0f);
		m_localTransform = glm::scale(m_localTransform, m_scale);
		m_localTransform = m_localTransform * glm::toMat4(m_rotation);

		glm::mat4x4 worldTransform = glm::identity<glm::mat4x4>();
		if (!m_parent.expired())
		{
			worldTransform = m_parent.lock()->GetWorldTransform();
		}

		glm::mat4 MMatrix = glm::mat4(1);
		glm::vec3 Scale = glm::vec3(1);
		glm::quat Orientation = glm::identity<glm::quat>();
		glm::vec3 Translation = glm::vec3(0);
		glm::vec3 Skew = glm::vec3(0);
		glm::vec4 Perspective = glm::vec4(0);
		glm::decompose(MMatrix,Scale,Orientation,Translation,Skew,Perspective);
		glm::vec4 relativeTranslation = glm::vec4(m_translation, 0) * Orientation;
		m_localTransform[3] = glm::vec4(relativeTranslation.x, relativeTranslation.y, relativeTranslation.z,1);
	}

	const glm::mat4x4& Node::GetWorldTransform()
	{
		if(m_typeOfNode == ROOT) 
		{
			UpdateLocalTransformMatrix();
			return m_localTransform;
		}

		if (m_transformDirty)
		{
			UpdateLocalTransformMatrix();
			if (!m_parent.expired())
			{
				m_worldTransform = m_parent.lock()->GetWorldTransform() * m_localTransform;
			}
			else 
			{
				m_worldTransform = m_localTransform;
			}
			m_transformDirty = false;
		}
		return m_worldTransform;
	}

	void Node::Translate(const glm::vec3& aTranslationVector)
	{
		m_translation += aTranslationVector;
		m_transformDirty = true;
	}

	void Node::Rotate(const glm::quat& rotationToApply)
	{
		glm::quat newOrientation = m_rotation * rotationToApply;
		m_rotation = newOrientation;
		m_transformDirty = true;
	}

	void Node::Rotate(const glm::vec3& eulerAngles)
	{
		Rotate(glm::quat(eulerAngles));
	}

	void Node::Scale(const glm::vec3& newScaleToApply)
	{
		m_scale *= newScaleToApply;
		m_transformDirty = true;
	}

	void Node::Update()
	{
		if (m_transformDirty)
		{
			for (auto& child : m_children)
			{
				child->SetTransformDirtyFlag();
			}

			UpdateLocalTransformMatrix();

			if(!m_parent.expired())
			{
				m_worldTransform = m_parent.lock()->GetWorldTransform() * m_localTransform;
			}
			else
			{
				m_worldTransform = m_localTransform;
			}
			m_transformDirty = false;
		}

		for (auto& child : m_children)
		{
			child->Update();
		}
	}

	glm::vec3 Node::GetTranslation()
	{
		return m_translation;
	}

	glm::vec3 Node::GetScale()
	{
		return m_scale;
	}

	glm::quat Node::GetRotation()
	{
		return m_rotation;
	}

	std::weak_ptr<Node> Node::GetParent()
	{
		return m_parent;
	}

	std::vector<std::shared_ptr<Node>>& Node::GetChildren()
	{
		return m_children;
	}

	void Node::AddChild(std::shared_ptr<Node> aNewChild)
	{
		m_children.push_back(aNewChild);
	}

	ENodeType Node::GetNodeType() const
	{
		return m_typeOfNode;
	}

	int Node::GetDataIndex() const
	{
		return m_dataIndex;
	}

	void Node::LookAt(glm::vec3 a_targetPos)
	{
		UpdateLocalTransformMatrix();
		m_localTransform = glm::lookAtRH(
			m_translation,
			a_targetPos,
			glm::vec3(m_localTransform[1].x, m_localTransform[1].y, m_localTransform[1].z));
	}

	Node* Node::GetActiveCamera(int cameraNode)
	{
		Node* node = nullptr;
		SearchTree(cameraNode, node);
		return node;
	}

	void Node::SearchTree(int nodeIndex, Node*& node)
	{
		if (node != nullptr)
			return;

		if (m_typeOfNode == CAMERA && m_dataIndex == nodeIndex)
		{
			node = this;
			return;
		}

		for(int i = 0; i < m_children.size(); i++)
		{
			m_children[i]->SearchTree(nodeIndex, node);
			if (node != nullptr)
				return;
		}
	}
}
