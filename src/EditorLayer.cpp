#include "EditorLayer.h"
#include <Renderer/Framebuffer.h>
//#include <imgui_demo.cpp>
//#include <Factory.h>
#include <Core/Log.h>

#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <Core/Application.h>

#include <Core/Input.h>
#include <Core/KeyCodes.h>

EditorLayer::EditorLayer() :
	Layer("SandboxLayer"),
	mBatchedGameObject(NULL)
{
	mUnlockedIcon = TS_ENGINE::Texture2D::Create("Assets\\Textures\\Gui\\Unlocked.png");
	mUnlockedIcon->SetVerticalFlip(false);
	mLockedIcon = TS_ENGINE::Texture2D::Create("Assets\\Textures\\Gui\\Locked.png");
	mLockedIcon->SetVerticalFlip(false);
	//mMeshFilterIcon = TS_ENGINE::Texture2D::Create("Assets\\Textures\\Gui\\MeshFilterIcon.png");
	//mMeshRendererIcon = TS_ENGINE::Texture2D::Create("Assets\\Textures\\Gui\\MeshRendererIcon.png");
	//mMaterialIcon = TS_ENGINE::Texture2D::Create("Assets\\Textures\\Gui\\MaterialIcon.png");
	//mLitMaterialIcon = TS_ENGINE::Texture2D::Create("Assets\\Textures\\Gui\\LitMaterialIcon.png");
}

void EditorLayer::OnAttach()
{
	mEditorCamera = CreateRef<TS_ENGINE::Camera>(TS_ENGINE::Camera::SCENECAMERA);

	float aspectRatio = (float)TS_ENGINE::Application::Get().GetWindow().GetWidth() / (float)TS_ENGINE::Application::Get().GetWindow().GetHeight();

	TS_ENGINE::Camera::Perspective perspective;
	perspective.fov = 45.0f;
	perspective.aspectRatio = aspectRatio;
	perspective.zNear = 0.1f;
	perspective.zFar = 1000.0f;
	mEditorCamera->SetPerspective(perspective);

	mCurrentScene = CreateRef<TS_ENGINE::Scene>("Gameplay");

	mLitShader = TS_ENGINE::Shader::Create("Lit", "Lit.vert", "Lit.frag");
	mBatchLitShader = TS_ENGINE::Shader::Create("BatchLit", "BatchLit.vert", "BatchLit.frag");

	mModelLoader = CreateRef<TS_ENGINE::ModelLoader>();

	for (uint32_t i = 0; i < mGridSizeX * mGridSizeY; i++)
		SpawnNewObject();

	mEditorCamera->SetPosition(105.280922f, 10.1856785f, 105.097504f);//For 50x50 Grid
	mEditorCamera->SetEulerAngles(0.308291346f, -0.783849120f, 0.0f);

	//mEditorCamera->SetPosition(65.4178528f, 1.79701405f, 65.4272217f);//For 317x317 Grid
	//mEditorCamera->SetEulerAngles(0.0597731248f, -0.787812710f, 0.0f);

	mCurrentShader = mLitShader;
	mCurrentShader->Bind();
}

Vector2 GetGridPosFromIndex(size_t index, size_t width)
{
	size_t x = index % width;
	size_t y = (index - x) / width;

	return Vector2(x, y);
}

void EditorLayer::SpawnNewObject()
{
	int randomIndex = 0 + (std::rand() % (6 - 0 + 1));
	int randomColorIndex = 0 + (std::rand() % (6 - 0 + 1));

	Ref<TS_ENGINE::Node> node = SpawnGameObjectNode(randomIndex, randomColorIndex);

	//go->SetColor(ColorPallete[randomColorIndex]);
	Vector2 gridPos = GetGridPosFromIndex(mNodes.size(), mGridSizeX);
	node->GetTransform()->SetPosition(2 * gridPos.x, 0, 2 * gridPos.y);

	mCurrentScene->GetSceneNode()->AddChild(node);
	node->SetParentNode(mCurrentScene->GetSceneNode());

	mNodes.push_back(node);
}

void EditorLayer::OnDetach()
{
	mCurrentScene.reset();
}

void EditorLayer::OnUpdate(float deltaTime)
{
	TS_ENGINE::Application::Get().ResetStats();

	mEditorCamera->OnUpdate(deltaTime);

	TS_ENGINE::RenderCommand::SetClearColor(Vector4(0.2f, 0.3f, 0.3f, 1.0f));
	TS_ENGINE::RenderCommand::Clear();

	mCurrentShader->SetMat4("u_View", mEditorCamera->GetViewMatrix());
	mCurrentShader->SetMat4("u_Projection", mEditorCamera->GetProjectionMatrix());

	mCurrentScene->Draw(mCurrentShader);
}

void EditorLayer::CreateUIForAllNodes(Ref<TS_ENGINE::Node> node)
{
	static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	static int selectedChildNodeIndex = -1;

	for (int i = 0; i < node->GetChildCount(); i++)
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		Ref<TS_ENGINE::Node> nodeChild = node->GetChildAt(i);

		ImGuiTreeNodeFlags node_flags = base_flags;

		if (nodeChild->GetChildCount() > 0)
		{
			bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)selectedChildNodeIndex, node_flags, nodeChild->GetName().c_str());

			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				selectedChildNodeIndex = i;

				mSelectedNode = nodeChild;
				//AddLog(LogCategory::info, TS_ENGINE::Utility::GetConcatCStrs("Clicked on: ", nodeChild->GetName()));
			}

			//HandleNodeDragDrop(mSelectedNode, nodeChild);

			if (node_open)
			{
				CreateUIForAllNodes(nodeChild);
				ImGui::TreePop();
			}
		}
		else//Tree Leaves
		{
			node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			ImGui::TreeNodeEx((void*)(intptr_t)selectedChildNodeIndex, node_flags, nodeChild->GetName().c_str());

			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				selectedChildNodeIndex = i;

				mSelectedNode = nodeChild;
				//AddLog(LogCategory::info, TS_ENGINE::Utility::GetConcatCStrs("Clicked on: ", nodeChild->GetName()));
			}

			//HandleNodeDragDrop(mSelectedNode, nodeChild);
		}
	}
}

void EditorLayer::OnImGUIRender()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	float minWinSizeX = style.WindowMinSize.x;
	style.WindowMinSize.x = minWinSizeX;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
	bool opened = false;

#pragma region Panel Size And Positions 
	//No need to set MainMenuBar size and position, DefaultSize: CurrentWindowWidth, 18.0f, DefaultPos: 0, 0

	ImVec2 inspectorPanelPos = ImVec2(TS_ENGINE::Application::Get().GetWindow().GetWidth() - 250, 0);
	ImVec2 inspectorPanelSize = ImVec2(250, TS_ENGINE::Application::Get().GetWindow().GetHeight() / 2);

	ImVec2 hierarchyPanelPos = ImVec2(TS_ENGINE::Application::Get().GetWindow().GetWidth() - 250, TS_ENGINE::Application::Get().GetWindow().GetHeight() / 2);
	ImVec2 hierarchyPanelSize = ImVec2(250, TS_ENGINE::Application::Get().GetWindow().GetHeight() / 2);

	ImVec2 viewportPanelPos = ImVec2(0, 0);
	ImVec2 viewportPanelSize = ImVec2(TS_ENGINE::Application::Get().GetWindow().GetWidth(), TS_ENGINE::Application::Get().GetWindow().GetHeight());
#pragma endregion

#pragma region Transform Gizmos
	ImGui::Begin("Viewport", &opened, window_flags | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

	ImGui::SetWindowPos("Viewport", viewportPanelPos);
	ImGui::SetWindowSize("Viewport", viewportPanelSize);

	//ImGuizmo set perspective and drawlist
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist();

	float windowWidth = (float)ImGui::GetWindowWidth();
	float windowHeight = (float)ImGui::GetWindowHeight();
	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

	const float* projection;
	const float* view;

	if (mEditorCamera)
	{
		projection = glm::value_ptr(mEditorCamera->GetProjectionMatrix());
		view = glm::value_ptr(mEditorCamera->GetViewMatrix());
	}

	const float* identityMatrix = glm::value_ptr(glm::mat4(1));

	//ImGui::Image((ImTextureID)colorBuffer, ImVec2{ windowWidth, windowHeight }, ImVec2(0, 1), ImVec2(1, 0));

	if (mSelectedNode)
	{
		float* matrix = (float*)glm::value_ptr(mSelectedNode->GetTransform()->GetModelMatrix());
		Vector3 initRot = mSelectedNode->GetTransform()->GetLocalEulerAngles();

		ImGuizmo::Manipulate(view, projection, mTransformOperation, mTransformMode, matrix);
		//std::string str = "Model matrix of " + std::string(mSelectedNode->GetName()) + " before change";
		//Logger::PrintMatrix(str.c_str(), matrix);

		//Utility::DecomposedData* dd =  Utility::Decompose(matrix);

		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);

		if (ImGuizmo::IsUsing())
		{
			if (mTranslateActive)
				mSelectedNode->GetTransform()->SetLocalPosition(Vector3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]));
			else if (mRotateActive)
				mSelectedNode->GetTransform()->SetLocalEulerAngles(Vector3(matrixRotation[0], matrixRotation[1], matrixRotation[2]));
			else if (mScaleActive)
				mSelectedNode->GetTransform()->SetLocalScale(Vector3(matrixScale[0], matrixScale[1], matrixScale[2]));

			mSelectedNode->UpdateModelMatrices();

			//mSelectedNode->GetTransform()->OverrideModelMatrix(matrix);
			//mSelectedNode->GetTransform()->ComputeModelMatrix();
			//ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);
		}
	}

	ImGui::End();
#pragma endregion

	ImGui::Begin("Stats");
	{
		ImGui::SetWindowSize(ImVec2(200, 150));
		ImGui::SetWindowPos(ImVec2(TS_ENGINE::Application::Get().GetWindow().GetWidth() - ImGui::GetWindowSize().x - 250, 0));

		if (ImGui::Checkbox("Batching enabled", &mCurrentScene->m_BatchingEnabled))
		{
			if (mCurrentScene->m_BatchingEnabled)
			{
				mSelectedNode = nullptr;
				mCurrentShader = mBatchLitShader;
			}
			else
			{
				mSelectedNode = nullptr;
				mCurrentScene->OnUnBatched();	

				for (auto& node : mNodes)
				{
					mCurrentScene->GetSceneNode()->AddChild(node);
					node->SetParentNode(mCurrentScene->GetSceneNode());
				}

				mCurrentShader = mLitShader;
			}

			mCurrentScene->m_BatchButton.Click(mCurrentShader, mNodes);
			
			if (mCurrentScene->m_BatchingEnabled)
				mCurrentScene->OnBatched();	
		}

		ImGui::Text("FPS: %.1f, %.3f ms/frame", 1000.0f / TS_ENGINE::Application::Get().GetDeltaTime(), TS_ENGINE::Application::Get().GetDeltaTime());

		ImGui::Text("Draw Calls: %d", TS_ENGINE::Application::Get().GetDrawCalls());
		ImGui::Text("Vertices: %d", TS_ENGINE::Application::Get().GetTotalVertices());
		ImGui::Text("Indices: %d", TS_ENGINE::Application::Get().GetTotalIndices());
	}
	ImGui::End();

	ImGui::Begin("Inspector");
	{
		ImGui::SetWindowSize(inspectorPanelSize);
		ImGui::SetWindowPos(inspectorPanelPos);

#pragma region Transform Component
		if (mSelectedNode != NULL)
		{
			ImGui::Checkbox(" ", &mSelectedNode->m_Enabled);
			ImGui::SameLine();
			ImGui::Text(mSelectedNode->GetName().c_str());

			ImGui::BeginChild("Transform", ImVec2(inspectorPanelSize.x - 18, 0.09259f * TS_ENGINE::Application::Get().GetWindow().GetWidth()), true, window_flags | ImGuiWindowFlags_NoScrollbar);

			ImGui::Text("Transform");
			ImGui::SameLine();
			if (ImGui::RadioButton("Tr", mTranslateActive))
			{
				mTranslateActive = true;
				mRotateActive = false;
				mScaleActive = false;

				//mTransformOperation = ImGuizmo::OPERATION::TRANSLATE;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Rt", mRotateActive))
			{
				mRotateActive = true;
				mTranslateActive = false;
				mScaleActive = false;

				//mTransformOperation = ImGuizmo::OPERATION::ROTATE;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Sc", mScaleActive))
			{
				mScaleActive = true;
				mTranslateActive = false;
				mRotateActive = false;

				//mTransformOperation = ImGuizmo::OPERATION::SCALE;
			}

			float* temp = new float[3]
			{
				mSelectedNode->GetTransform()->GetLocalPosition().x,
					mSelectedNode->GetTransform()->GetLocalPosition().y,
					mSelectedNode->GetTransform()->GetLocalPosition().z
			};

			if (ImGui::DragFloat3("Position", temp))
			{
				mSelectedNode->GetTransform()->SetLocalPosition(Vector3(temp[0], temp[1], temp[2]));
				//mSelectedNode->UpdateModelMatrices();
			}

			temp = new float[3] 
			{
				mSelectedNode->GetTransform()->GetLocalEulerAngles().x,
					mSelectedNode->GetTransform()->GetLocalEulerAngles().y,
					mSelectedNode->GetTransform()->GetLocalEulerAngles().z
			};

			if (ImGui::DragFloat3("Rotation", temp))
			{
				mSelectedNode->GetTransform()->SetLocalEulerAngles(Vector3(temp[0], temp[1], temp[2]));
				//mSelectedNode->UpdateModelMatrices();
			}
			//Scale
			mCurrScale[0] = mSelectedNode->GetTransform()->GetLocalScale().x;
			mCurrScale[1] = mSelectedNode->GetTransform()->GetLocalScale().y;
			mCurrScale[2] = mSelectedNode->GetTransform()->GetLocalScale().z;

			if (ImGui::DragFloat3("Scale", mCurrScale))
			{
				//Avoid zero
				if (mCurrScale[0] == 0.0f)
					mCurrScale[0] = 0.0001f;
				if (mCurrScale[1] == 0.0f)
					mCurrScale[1] = 0.0001f;
				if (mCurrScale[2] == 0.0f)
					mCurrScale[2] = 0.0001f;

				//Avoid zero
				if (mLastScale[0] == 0.0f)
					mLastScale[0] = 0.0001f;
				if (mLastScale[1] == 0.0f)
					mLastScale[1] = 0.0001f;
				if (mLastScale[2] == 0.0f)
					mLastScale[2] = 0.0001f;

				if (m_ScaleLock)
				{
					//Logger::Print("Temp Scale", mCurrScale);
					//Logger::Print("Last Scale", mLastScale);

					if (mCurrScale[0] != mLastScale[0])//X Scaled
					{
						mCurrScale[1] = mLastScale[1] * (mCurrScale[0] / mLastScale[0]);
						mCurrScale[2] = mLastScale[2] * (mCurrScale[0] / mLastScale[0]);
					}
					if (mCurrScale[1] != mLastScale[1])//Y Scaled
					{
						mCurrScale[0] = mLastScale[0] * (mCurrScale[1] / mLastScale[1]);
						mCurrScale[2] = mLastScale[2] * (mCurrScale[1] / mLastScale[1]);
					}
					if (mCurrScale[2] != mLastScale[2])//Z Scaled
					{
						mCurrScale[0] = mLastScale[0] * (mCurrScale[2] / mLastScale[2]);
						mCurrScale[1] = mLastScale[1] * (mCurrScale[2] / mLastScale[2]);
					}
				}
				//Record last scale
				mLastScale[0] = mCurrScale[0];
				mLastScale[1] = mCurrScale[1];
				mLastScale[2] = mCurrScale[2];

				mSelectedNode->GetTransform()->SetLocalScale(Vector3(mCurrScale[0], mCurrScale[1], mCurrScale[2]));
				mSelectedNode->UpdateModelMatrices();
			}

			ImGui::SameLine();
			//ImGui::Checkbox("L", &mScaleLock);		

			if (!m_ScaleLock)
			{
				if (ImGui::ImageButton((ImTextureID)mUnlockedIcon->GetRendererID(), ImVec2(18, 18)))
					m_ScaleLock = true;
			}
			else
			{
				if (ImGui::ImageButton((ImTextureID)mLockedIcon->GetRendererID(), ImVec2(18, 18)))
					m_ScaleLock = false;
			}

			//temp = NULL;
			delete temp;
			ImGui::EndChild();
		}

#pragma endregion
	
	}
	ImGui::End();

	ImGui::Begin("Hierarchy");
	{
		ImGui::SetWindowSize(hierarchyPanelSize);
		ImGui::SetWindowPos(hierarchyPanelPos);

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

		if (ImGui::TreeNodeEx((void*)(intptr_t)0, base_flags, mCurrentScene->GetSceneNode()->GetName().c_str()))
		{
			CreateUIForAllNodes(mCurrentScene->GetSceneNode());
			ImGui::TreePop();
		}
	}
	ImGui::End();
}

void EditorLayer::OnEvent(TS_ENGINE::Event& e)
{
	TS_ENGINE::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<TS_ENGINE::KeyPressedEvent>(TS_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
	dispatcher.Dispatch<TS_ENGINE::MouseButtonPressedEvent>(TS_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
}

bool EditorLayer::OnKeyPressed(TS_ENGINE::KeyPressedEvent& e)
{
	switch (e.GetKeyCode())
	{
	case TS_ENGINE::Key::Escape:
		TS_ENGINE::Application::Get().Close();
		break;
	case TS_ENGINE::Key::Tab:
		TS_ENGINE::Application::Get().ToggleWireframeMode();
		break;
	case TS_ENGINE::Key::G:
		SpawnNewObject();
		break;
	case TS_ENGINE::Key::Delete:
		break;
		break;
	}

	return false;
}

bool EditorLayer::OnMouseButtonPressed(TS_ENGINE::MouseButtonPressedEvent& e)
{
	return false;
}

void EditorLayer::OnOverlay()
{

}

Ref<TS_ENGINE::Node> EditorLayer::SpawnGameObjectNode(uint32_t index, uint32_t randColorIndex)//Index is a random number between 0 to 6
{
	if (index == 0)
	{
		Ref<TS_ENGINE::Node> node = CreateRef<TS_ENGINE::Node>();
		node->SetName("QuadNode");
		node->GetTransform()->SetEulerAngles(0.0f, 180.0f, 0.0f);

		Ref<TS_ENGINE::Quad> quad = CreateRef<TS_ENGINE::Quad>("Quad");
		quad->SetColor(ColorPallete[randColorIndex]);
		quad->SetTexture("Crate.png");
		quad->Create();

		node->AttachGameObject(quad);
		return node;
	}
	else if (index == 1)
	{
		Ref<TS_ENGINE::Node> node = CreateRef<TS_ENGINE::Node>();
		node->SetName("QuadNode");
		node->GetTransform()->SetEulerAngles(0.0f, 180.0f, 0.0f);

		Ref<TS_ENGINE::Quad> quad = CreateRef<TS_ENGINE::Quad>("Quad");
		quad->SetColor(ColorPallete[randColorIndex]);
		quad->SetTexture("CrateTex1.png");
		quad->Create();


		node->AttachGameObject(quad);
		return node;
	}
	else if (index == 2)
	{
		Ref<TS_ENGINE::Node> node = CreateRef<TS_ENGINE::Node>();
		node->SetName("QuadNode");
		node->GetTransform()->SetEulerAngles(0.0f, 180.0f, 0.0f);

		Ref<TS_ENGINE::Quad> quad = CreateRef<TS_ENGINE::Quad>("Quad");
		quad->SetColor(ColorPallete[randColorIndex]);
		quad->SetTexture("Terrain.png");
		quad->Create();

		node->AttachGameObject(quad);
		return node;
	}
	else if (index == 3)
	{
		Ref<TS_ENGINE::Node> node = CreateRef<TS_ENGINE::Node>();
		node->SetName("CubeNode");
		node->GetTransform()->SetEulerAngles(0, 180.0f + 90.0f, 0);

		Ref<TS_ENGINE::Cube> cube = CreateRef<TS_ENGINE::Cube>("Cube");
		cube->SetColor(ColorPallete[randColorIndex]);
		cube->SetTexture("Crate.png");
		cube->Create();

		node->AttachGameObject(cube);
		return node;
	}
	else if (index == 4)
	{
		Ref<TS_ENGINE::Node> node = CreateRef<TS_ENGINE::Node>();
		node->SetName("CubeNode");
		node->GetTransform()->SetEulerAngles(0.0f, 180.0f + 45.0f, 0.0f);

		Ref<TS_ENGINE::Cube> cube = CreateRef<TS_ENGINE::Cube>("Cube");
		cube->SetColor(ColorPallete[randColorIndex]);
		cube->SetTexture("Crate.png");
		cube->Create();

		node->AttachGameObject(cube);
		return node;
	}
	else if (index == 5)
	{
		Ref<TS_ENGINE::Node> node = CreateRef<TS_ENGINE::Node>();
		node->SetName("Model");
		node->GetTransform()->SetEulerAngles(-90, 0, 180.0f);
		node->GetTransform()->SetScale(0.5f);

		mModelLoader->LoadModel("Assets\\Models", "monk_character.glb");
		mModelLoader->GetLastLoadedModel()->SetName("Model1");

		node->AttachGameObject(mModelLoader->GetLastLoadedModel());
		return node;
	}
	else if (index == 6)
	{
		Ref<TS_ENGINE::Node> node = CreateRef<TS_ENGINE::Node>();
		node->SetName("Model");
		node->GetTransform()->SetEulerAngles(-90, 0, 180.0f);
		node->GetTransform()->SetScale(0.5f);

		mModelLoader->LoadModel("Assets\\Models", "monk_character.glb");
		mModelLoader->GetLastLoadedModel()->SetName("Model2");

		node->AttachGameObject(mModelLoader->GetLastLoadedModel());
		return node;
	}
	else
		return nullptr;
}
