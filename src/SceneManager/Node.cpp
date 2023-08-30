#include "tspch.h"
#include "SceneManager/Node.h"

#ifdef TS_ENGINE_EDITOR
#include <imgui.h>
#endif // TS_ENGINE_EDITOR

namespace TS_ENGINE
{
	Node::Node()
	{
		this->mIsInitialized = false;
		//this->mName = "";
		//this->mNodeType = Type::EMPTY;
		this->mTransform = CreateRef<Transform>();
		this->mParentNode = nullptr;
		this->mMeshes = {};
#ifdef TS_ENGINE_EDITOR
		this->mIsVisibleInEditor = true;
#endif
	}

	Node::~Node()
	{
		Destroy();//TODO:: Why is it getting destructured
	}

	void Node::Destroy()
	{
		m_Enabled = false;
		mParentNode->RemoveChild(mNodeRef);

		mMeshes.clear();

		//mName = "";
		mTransform.reset();

		for (auto& node : mChildren)
		{
			node.reset();
		}


		mParentNode.reset();

		mChildren.clear();
	}

	Ref<Node> Node::Duplicate()
	{
		Ref<Node> duplicateNode = CreateRef<Node>();	
		duplicateNode->mNodeRef = duplicateNode;		
		duplicateNode->mNodeRef->mMeshes = mNodeRef->mMeshes;
		
		duplicateNode->mNodeRef->mTransform = CreateRef<Transform>();
		duplicateNode->mNodeRef->mTransform->m_Pos = mNodeRef->mTransform->m_Pos;
		duplicateNode->mNodeRef->mTransform->m_EulerAngles = mNodeRef->mTransform->m_EulerAngles;
		duplicateNode->mNodeRef->mTransform->m_Scale = mNodeRef->mTransform->m_Scale;
		
#ifdef TS_ENGINE_EDITOR
		duplicateNode->mNodeRef->mIsVisibleInEditor = mNodeRef->mIsVisibleInEditor;
#endif

		duplicateNode->mParentNode = mNodeRef->mParentNode;

		for (auto& child : mNodeRef->mChildren)
		{
			duplicateNode->AddChild(child->Duplicate());
		}

		duplicateNode->mNodeRef->Initialize(mNodeRef->mEntity->GetName(), mNodeRef->mEntity->GetEntityType());
		duplicateNode->mNodeRef->UpdateSiblings();
		return duplicateNode;
	}

	/*void Node::SetEntityType(EntityType entityType)
	{
		mEntityType = entityType;
	}*/

	void Node::SetNodeRef(Ref<Node> node)
	{
		mNodeRef = node;
	}

	/*void Node::SetNodeType(const Type& nodeType)
	{
		mNodeType = nodeType;
	}*/

	//void Node::SetName(const std::string& name)
	//{
	//	//TS_CORE_TRACE("Renamed Node with entityID {0} to {1}", mEntity->GetEntityID(), name);
	//	//mName = name;		
	//	mEntity->SetName(name);
	//}

	void Node::SetParent(Ref<Node> parentNode)
	{
		//TS_CORE_INFO("Setting parent of {0} as {1}", mEntity->GetName().c_str(), parentNode->mEntity->GetName().c_str());

		if (parentNode)
		{
			if (mNodeRef->mParentNode)
			{
				mNodeRef->mParentNode->RemoveChild(mNodeRef);
			}

			parentNode->AddChild(mNodeRef);
			mNodeRef->GetTransform()->ComputeTransformationMatrix(parentNode);
		}
	}

	void Node::ChangeParent(Ref<Node> parentNode)
	{
		//TS_CORE_INFO("Setting parent of {0} as {1}", mEntity->GetName().c_str(), parentNode->mEntity->GetName().c_str());

		if (parentNode)
		{
			if (mNodeRef->mParentNode)
			{
				mNodeRef->mParentNode->RemoveChild(mNodeRef);
			}

			parentNode->AddChild(mNodeRef);			
		}
	}

	void Node::SetPosition(float* pos)
	{
		mTransform->SetLocalPosition(pos);

		mTransform->ComputeTransformationMatrix(mParentNode);

		for (auto& child : mChildren)
			child->InitializeTransformMatrices();
	}
	void Node::SetPosition(float x, float y, float z)
	{
		mTransform->SetLocalPosition(x, y, z);

		mTransform->ComputeTransformationMatrix(mParentNode);

		for (auto& child : mChildren)
			child->InitializeTransformMatrices();
	}
	void Node::SetPosition(const Vector3& pos)
	{
		mTransform->SetLocalPosition(pos);

		mTransform->ComputeTransformationMatrix(mParentNode);

		for (auto& child : mChildren)
			child->InitializeTransformMatrices();
	}

	void Node::SetEulerAngles(float* eulerAngles)
	{
		mTransform->SetLocalEulerAngles(eulerAngles);

		mTransform->ComputeTransformationMatrix(mParentNode);

		for (auto& child : mChildren)
			child->InitializeTransformMatrices();
	}
	void Node::SetEulerAngles(float x, float y, float z)
	{
		mTransform->SetLocalEulerAngles(x, y, z);

		mTransform->ComputeTransformationMatrix(mParentNode);

		for (auto& child : mChildren)
			child->InitializeTransformMatrices();
	}
	void Node::SetEulerAngles(const Vector3& eulerAngles)
	{
		mTransform->SetLocalEulerAngles(eulerAngles);

		mTransform->ComputeTransformationMatrix(mParentNode);

		for (auto& child : mChildren)
			child->InitializeTransformMatrices();
	}

	void Node::SetScale(float* scale)
	{
		mTransform->SetLocalScale(scale);

		mTransform->ComputeTransformationMatrix(mParentNode);

		for (auto& child : mChildren)
			child->InitializeTransformMatrices();
	}
	void Node::SetScale(float x, float y, float z)
	{
		mTransform->SetLocalScale(x, y, z);

		mTransform->ComputeTransformationMatrix(mParentNode);

		for (auto& child : mChildren)
			child->InitializeTransformMatrices();
	}
	void Node::SetScale(const Vector3& scale)
	{
		mTransform->SetLocalScale(scale);

		mTransform->ComputeTransformationMatrix(mParentNode);

		for (auto& child : mChildren)
			child->InitializeTransformMatrices();
	}

	void Node::AddChild(Ref<Node> child)
	{
		child->mParentNode = mNodeRef;
		mChildren.push_back(child);
		//TS_CORE_INFO("{0} is set as child of {1}", child->mEntity->GetName().c_str(), mNodeRef->mEntity->GetName().c_str());

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
		{
			//child.reset();
			delete& child;
		}

		mChildren.clear();
	}

	Ref<Node> Node::GetChildAt(uint32_t childIndex) const
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
				if (child != mNodeRef)
					mSiblings.push_back(child);
			}
		}
		else
		{
			TS_CORE_ERROR("There is no parent for {0}", mNodeRef->mEntity->GetName().c_str());
		}
	}

	void Node::InitializeTransformMatrices()
	{
		mTransform->ComputeTransformationMatrix(mParentNode);

		for (auto& child : mChildren)
		{
			child->InitializeTransformMatrices();
		}
	}

	void Node::UpdateTransformationMatrices(Matrix4 transformationMatrix)
	{
		mTransform->m_TransformationMatrix = transformationMatrix;

		for (auto& child : mChildren)
		{
			child->InitializeTransformMatrices();
		}
	}

	void Node::Initialize(const std::string& name, const EntityType& entityType)
	{		
		mNodeRef->mEntity = EntityManager::GetInstance()->Register(name, entityType);
		InitializeTransformMatrices();
		
		mIsInitialized = true;
	}

	//If there is no parent set parentTransformModelMatrix to identity
	void Node::Update(Ref<Shader> shader, float deltaTime)
	{
		TS_CORE_ASSERT(mIsInitialized, "Node is not initialized!");

		//Send modelMatrix to shader
		shader->SetMat4("u_Model", mTransform->GetTransformationMatrix());

		if (m_Enabled)
		{
			//Draw Meshes
			for (auto& mesh : mMeshes)
			{
				mesh->Render(mEntity->GetEntityID());
			}

			//Send children modelMatrix to shader and draw gameobject with attached to child
			for (auto& child : mChildren)
			{
				child->Update(shader, deltaTime);
			}
		}
	}

	void Node::LookAt(Ref<Node> targetNode)
	{
		mTransform->LookAt(mParentNode, targetNode->GetTransform());
	}

	void Node::AddMesh(Ref<Mesh> mesh)
	{
		mMeshes.push_back(mesh);
	}

	void Node::AddMeshes(std::vector<Ref<Mesh>> meshes)
	{
		mMeshes = meshes;
	}

	void Node::PrintChildrenName()
	{
		TS_CORE_TRACE("Node {0} has children named: ", mEntity->GetName().c_str());

		for (auto& child : mChildren)
		{
			TS_CORE_TRACE("{0} ", child->mEntity->GetName().c_str());
			child->PrintChildrenName();
		}
	}

#ifdef TS_ENGINE_EDITOR
	void Node::HideInEditor()
	{
		mIsVisibleInEditor = false;
	}
#endif
}