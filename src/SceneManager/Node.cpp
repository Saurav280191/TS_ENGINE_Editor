#include "tspch.h"
#include "SceneManager/Node.h"

namespace TS_ENGINE
{
	Node::Node() : 
		mParentNode(nullptr),
		mTransform(CreateRef<Transform>()),
#ifdef TS_ENGINE_EDITOR
		mIsVisibleInEditor(true)
#endif
	{

	}

	Node::~Node()
	{
		Destroy();
	}

	void Node::Destroy()
	{
		mName = "";
		mTransform.reset();
		mAttachedObject.reset();	

		for (auto& node : mChildren)
			node.reset();

		mParentNode = nullptr;

		mChildren.clear();
	}

	void Node::SetName(std::string name)
	{
		mName = name;
	}

	void Node::AttachObject(Ref<Object> object)
	{
		mAttachedObject = object;
	}

	void Node::SetNodeRef(Ref<Node> node)
	{
		mNodeRef = node;
	}

	void Node::SetParent(Ref<Node> parentNode)
	{
		SetParent(parentNode.get());
	}

	void Node::SetParent(Node* parentNode)
	{
		TS_CORE_INFO("Setting parent of {0} as {1}", mNodeRef->GetName(), parentNode->GetName());

		if (mNodeRef->mParentNode)
			mNodeRef->mParentNode->RemoveChild(mNodeRef);

		parentNode->AddChild(mNodeRef);
	}

	void Node::AddChild(Ref<Node> child)
	{
		child->mParentNode = this; 
		mChildren.push_back(child);
		TS_CORE_INFO(child->GetName() + " is set as child of " + this->GetName());

		child->UpdateSiblings();
	}

	void Node::RemoveChild(Ref<Node> child)
	{
		mChildren.erase(std::remove(mChildren.begin(), mChildren.end(), child), mChildren.end());
		child->UpdateSiblings();
	}

	void Node::RemoveAllChildren()
	{
		for (auto& child : mChildren)
			child.reset();

		mChildren.clear();
	}

	void Node::SetEntityType(EntityType entityType)
	{
		mEntityType = entityType;
	}

	void Node::LookAt(Ref<Node> targetNode)
	{
		mTransform->LookAt(mParentNode, targetNode->GetTransform());
	}

	const Ref<Node> Node::GetChildAt(uint32_t childIndex) const
	{
		try
		{
			Ref<Node> child = mChildren[childIndex];
			return child;
		}
		catch (std::out_of_range& e)
		{
			TS_CORE_ERROR(e.what());
		}
	}
	
	void Node::UpdateSiblings()
	{
		if (mParentNode)
		{
			mSiblings = {};

			for (auto& child : mParentNode->mChildren)
			{
				if (child != this->mNodeRef)
					mSiblings.push_back(child);
			}
		}
		else
		{
			TS_CORE_ERROR("There is not parent for " + this->GetName());
		}
	}

	/// <summary>
	/// Updates Model matrix for it's self and for children
	/// </summary>
	void Node::InitializeTransformMatrices()
	{
		mTransform->ComputeTransformationMatrix(this, mParentNode);

		for (auto& child : mChildren)		
			child->InitializeTransformMatrices();		
	}

	void Node::SetPosition(float* pos)
	{
		mTransform->SetLocalPosition(pos);

		mTransform->ComputeTransformationMatrix(this, mParentNode);

		for (auto& child : mChildren)
			child->InitializeTransformMatrices();
	}

	void Node::SetEulerAngles(float* eulerAngles)
	{
		mTransform->SetLocalEulerAngles(eulerAngles);

		mTransform->ComputeTransformationMatrix(this, mParentNode);

		for (auto& child : mChildren)
			child->InitializeTransformMatrices();
	}

	void Node::SetScale(float* scale)
	{
		mTransform->SetLocalScale(scale);

		mTransform->ComputeTransformationMatrix(this, mParentNode);

		for (auto& child : mChildren)
			child->InitializeTransformMatrices();
	}

	void Node::UpdateTransformationMatrices(Matrix4 transformationMatrix)
	{
		mTransform->SetTransformationMatrix(transformationMatrix);

		for (auto& child : mChildren)
			child->InitializeTransformMatrices();
	}

	//If there is no parent set parentTransformModelMatrix to identity
	void Node::Update(Ref<Shader> shader, float deltaTime)
	{
		//Send modelMatrix to shader
		shader->SetMat4("u_Model", mTransform->GetTransformationMatrix());
		
		//Draw GameObject
		if (mAttachedObject)
		{
			shader->SetInt("u_EntityID", mAttachedObject->GetEntityID());
			mAttachedObject->Update(shader, deltaTime);
		}

		//Send children modelMatrix to shader and draw gameobject with attached to child
		for (auto& child : mChildren)		
			child->Update(shader, deltaTime);		
	}
}