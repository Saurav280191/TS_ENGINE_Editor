#include "EditorLayer.h"
//#include <imgui_demo.cpp>
#include <Factory.h>
#include <Core/Log.h>

#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <Core/Application.h>

#include <Core/Input.h>
#include <Core/KeyCodes.h>
#include <Utils/Utility.h>
#include <Platform/OpenGL/OpenGLVertexArray.h>

#include "Renderer/MaterialManager.h"

ImGuiWindowFlags EditorLayer::mDefaultWindowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

EditorLayer::EditorLayer() :
	Layer("SandboxLayer"),
	//mBatchedGameObject(NULL)
	mIsControlPressed(false),
	mDeltaTime(0.0f),
	mOrthographicProjectionActive(false)
{

}

void EditorLayer::OnAttach()
{
	mSceneGui = CreateRef<TS_ENGINE::SceneGui>();

	// TODO: Modify code to pass materials instead of shaders and handle the binding in MaterialManager itself

	// Activate Shader
	mCurrentShader = TS_ENGINE::MaterialManager::GetInstance()->GetUnlitMaterial()->GetShader();
	mCurrentShader->Bind();
}

void EditorLayer::OnDetach()
{
	if (TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene())
	{
		TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene().reset();
	}
}

void EditorLayer::OnUpdate(float deltaTime)
{
	mDeltaTime = deltaTime;																								// DeltaTime

	TS_ENGINE::Application::GetInstance().ResetStats();																	// Reset Stats

	if (TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene())
	{
		TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->Update(deltaTime);									// Update Current Scene
		TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->Render(mCurrentShader, deltaTime);					// Render Current Scene
	}

	if (mSceneGui->IsViewportActiveWindow && !mIsControlPressed && !mSceneGui->m_ShowNewSceneWindow)
	{
		if (TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene())
		{
			TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetEditorCamera()->Controls(deltaTime);			// Editor Camera Controls
		}

	}

	// Editor camera pass
	{
		if (TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene())
		{
			TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->UpdateCameraRT(TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetEditorCamera(), mCurrentShader, deltaTime, true);

			// Render after all gameObjects rendered to show as an overlay.
			OnOverlayRender();																							// GUI Render

			if (!ImGuizmo::IsOver() && !mSceneGui->m_ShowNewSceneWindow)
			{
				PickGameObject();																						// Picking
			}

			TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetEditorCamera()->GetFramebuffer()->Unbind();	// Unbind Framebuffer
		}
	}
}

void EditorLayer::PickNode(Ref<TS_ENGINE::Node> node, int entityID)
{
	//if (node->GetMeshes().size() > 0)
	{
		//TS_CORE_TRACE("Checking: {0}", node->GetName());

		if (node->GetEntity()->GetEntityID() == entityID)
		{
			TS_CORE_TRACE("{0} has matching entityID", node->GetEntity()->GetName().c_str());
			mMatchingNode = node;
		}
	}

	for (auto& childNode : node->GetChildren())
	{
		PickNode(childNode, entityID);
	}
}

Ref<TS_ENGINE::Node> EditorLayer::PickNodeByEntityID(int entityID)
{
	if (TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene())
	{
		// Check all node in the scene hierarchy
		PickNode(TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode(), entityID);

		// Check all bone nodes from factory class's instantiated models
		for (auto& nameNodeModelPair : TS_ENGINE::Factory::GetInstance()->mLoadedModelNodeMap)
		{
			std::pair<Ref<TS_ENGINE::Node>, Ref<TS_ENGINE::Model>> nodeModelPair = nameNodeModelPair.second;	// Node & Model Pair
			Ref<TS_ENGINE::Model> model = nodeModelPair.second;													// Model
			
			for (auto& [name, bone] : model->GetBoneInfoMap())
			{
				if (bone)
				{				
					if (bone->PickNode(entityID))
					{
						Ref<TS_ENGINE::Node> node = bone->GetNode();											// Node for Bone Gui
						mMatchingNode = node;
					}
				}	
			}
		}
	}
	return mMatchingNode;
}

void EditorLayer::PickGameObject()
{
	// If left mouse button is pressed
	if (TS_ENGINE::Input::IsMouseButtonPressed(TS_ENGINE::Mouse::Button0) && !mMouseClicked)
	{
		// Picking code
		auto& [mx, my] = ImGui::GetMousePos();
		
		mx -= mSceneGui->GetViewportImageRect()->x;
		my -= mSceneGui->GetViewportImageRect()->y;

		my = mSceneGui->GetViewportImageRect()->h - my;

		int mouseX = (int)mx;	// TODO: Offset (-8, 38) is needed for proper picking. Need to find the root cause of this issue.
		int mouseY = (int)my;	// This is probably because the viewport has borders

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)mSceneGui->GetViewportImageRect()->w && mouseY < (int)mSceneGui->GetViewportImageRect()->h)
		{
			//Vector4 pixelColor = mEditorCamera->GetFramebuffer()->ReadPixelColor(0, mouseX, mouseY);
			//mPickedColor = ImVec4(pixelColor.x / 255.0f, pixelColor.y / 255.0f, pixelColor.z / 255.0f, pixelColor.z / 255.0f);
			//TS_CORE_TRACE("Pixel color = {0}", glm::to_string(pixelColor));

			int entityID = TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetEditorCamera()->GetFramebuffer()->ReadPixel(1, mouseX, mouseY);

			// Print entity ID
			TS_CORE_TRACE("Picking entity with ID : {0}", entityID);

			if (entityID != -1)
			{
				Ref<TS_ENGINE::Node> hoveredOnNode = nullptr;

				if (entityID != TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetSkyboxEntityID())// Avoid skybox selection
				{
					bool clickedOnSceneCamera = false;
					Ref<TS_ENGINE::SceneCamera> clickedSceneCamera = nullptr;
					for (auto& sceneCamera : TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetSceneCameras())
					{
						if (sceneCamera->IsSceneCameraGuiSelected(entityID))
						{
							clickedOnSceneCamera = true;
							clickedSceneCamera = sceneCamera;
							break;
						}
					}

					//Check if scene camera's GUI was selected
					if (clickedOnSceneCamera)
						hoveredOnNode = clickedSceneCamera->GetNode();
					else
						hoveredOnNode = PickNodeByEntityID(entityID);

					if (hoveredOnNode != nullptr)
					{
						mSceneGui->SetSelectedNode(hoveredOnNode);
					}
				}
				else// If skybox was selected
				{
					mSceneGui->SetSelectedNode(nullptr);//Deselect
				}
			}
			else
			{
				mSceneGui->SetSelectedNode(nullptr);//Deselect
			}
		}

		mMouseClicked = true;
	}

	if (TS_ENGINE::Input::IsMouseButtonReleased(TS_ENGINE::Mouse::Button0) && mMouseClicked)
	{
		mMouseClicked = false;
	}
}

#pragma region ImGUI functions

void EditorLayer::OnImGuiRender()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	float minWinSizeX = style.WindowMinSize.x;
	style.WindowMinSize.x = minWinSizeX;
	style.WindowBorderSize = 1.0f;

	mDefaultWindowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

	ShowMainMenuBar();

	if (ImGui::DockSpaceOverViewport())
	{
		//ShowSubMenu();
		ShowPanels();
	}
}

void EditorLayer::ShowMainMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("NewScene", "CTRL+N"))
			{
				if (!mSceneGui->m_ShowNewSceneWindow)
					mSceneGui->m_ShowNewSceneWindow = true;				
			}

			if (ImGui::MenuItem("Save", "CTRL+S"))
			{
				if (TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene())
				{
					TS_ENGINE::SceneManager::GetInstance()->SaveCurrentScene();
					std::string sceneName = TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode()->GetEntity()->GetName();
					mSceneGui->TakeSnapshot(TS_ENGINE::Application::s_ThumbnailsDir.string() + "\\" + sceneName + ".png");
				}
				else
				{
					TS_CORE_ERROR("Current scene is not set!");
				}
			}

			if (ImGui::MenuItem("Close"))
			{
				TS_ENGINE::SceneManager::GetInstance()->FlushCurrentScene();
			}
			ImGui::EndMenu();
		}

		/*if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z"))
			{

			}

			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false))
			{

			}

			ImGui::Separator();

			if (ImGui::MenuItem("Cut", "CTRL+X"))
			{

			}

			if (ImGui::MenuItem("Copy", "CTRL+C"))
			{

			}

			if (ImGui::MenuItem("Paste", "CTRL+V"))
			{

			}

			ImGui::EndMenu();
		}*/

		if (TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene())
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::BeginMenu("GameObject"))
				{
					if (ImGui::MenuItem("Quad"))
					{
						TS_ENGINE::Factory::GetInstance()->InstantiateQuad("New Quad", TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode());
					}

					if (ImGui::MenuItem("Cube"))
					{
						TS_ENGINE::Factory::GetInstance()->InstantiateCube("New Cube", TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode());
					}

					if (ImGui::MenuItem("Sphere"))
					{
						TS_ENGINE::Factory::GetInstance()->InstantiateSphere("New Sphere", TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode());
					}

					if (ImGui::MenuItem("Cylinder"))
					{
						TS_ENGINE::Factory::GetInstance()->InstantiateCylinder("New Cylinder", TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode());
					}

					if (ImGui::MenuItem("Cone"))
					{
						TS_ENGINE::Factory::GetInstance()->InstantiateCone("New Cone", TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode());
					}

					if (ImGui::MenuItem("Empty"))
					{
						TS_ENGINE::Factory::GetInstance()->InstantitateEmptyNode("Empty GameObject", TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode());
					}

					/*if (ImGui::MenuItem("Model"))
					{
						TS_ENGINE::Factory::GetInstance()->CreateGameObject(TS_ENGINE::PrimitiveType::MODEL);
					}*/

					ImGui::EndMenu();
				}
				
				if (ImGui::BeginMenu("Light"))
				{
					if (ImGui::MenuItem("Directional"))
					{

					}

					if (ImGui::MenuItem("Point"))
					{

					}

					if (ImGui::MenuItem("Spot"))
					{

					}
					ImGui::EndMenu();
				}
				
				ImGui::EndMenu();
			}
		}

		ImGui::EndMainMenuBar();
	}
}

void EditorLayer::ShowSubMenu()
{
	//No need to set MainMenuBar size and position, DefaultSize: CurrentWindowWidth, 18.0f, DefaultPos: 0, 0
	ImVec2 subMenuPos = ImVec2(0.0f, 19.8f);
	ImVec2 subMenuSize = ImVec2((float)TS_ENGINE::Application::GetInstance().GetWindow().GetWidth() - 250.0f, 34.5f);

#pragma region Sub-Menu 
	//ImGui::SetNextWindowPos(subMenuPos);
	//ImGui::SetNextWindowSize(subMenuSize);
	ImGui::Begin("SubMenu");// , 0, defaultWindowFlags | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar);
	{
		ImGui::Button("Scene", ImVec2(50, 20));
		ImGui::SameLine();
		ImGui::Button("Game", ImVec2(50, 20));

		{
			//ImGui::Checkbox("Show Grid", &mShowGrid);

			//ImGui::SameLine();
			//ImGui::Spacing();
			//ImGui::SameLine();

			//ImGui::Checkbox("Gamma Correction", &mGammaCorrection);

			//ImGui::SameLine();
			//ImGui::Spacing();
			//ImGui::SameLine();

			//if (mGammaCorrection)
			//{
			//	ImGui::PushItemWidth(200);
			//	ImGui::SliderFloat("GammaValue", &mGammaValue, 0.1f, 2.2f);
			//}

			//ImGui::SameLine();
			//ImGui::Spacing();
			//ImGui::SameLine();

			//ImGui::Checkbox("HDR", &mHdr);

			//ImGui::SameLine();
			//ImGui::Spacing();
			//ImGui::SameLine();

			//if (mHdr)
			//{
			//	ImGui::PushItemWidth(200);
			//	ImGui::SliderFloat("Expose", &mHdrExposure, 0.01f, 10.0f);//More expose will hide the detail
			//}
		}
	}

	ImGui::End();
#pragma endregion
}

void EditorLayer::ShowPanels()
{
#pragma region Panel Size And Positions 
	ImVec2 statsPanelPos = ImVec2(TS_ENGINE::Application::GetInstance().GetWindow().GetWidth() - 450.0f, 19.0f);
	ImVec2 statsPanelSize = ImVec2(200.0f, 150.0f);
#pragma endregion

	mSceneGui->IsViewportActiveWindow = false;
	mSceneGui->ShowViewportWindow();
	mSceneGui->ShowInspectorWindow();
	mSceneGui->ShowHierarchyWindow();
	mSceneGui->ShowContentBrowser();
	mSceneGui->ShowAnimationPanel();

	if (mSceneGui->m_ShowNewSceneWindow)
	{
		mSceneGui->ShowNewSceneWindow();
	}
}

#pragma endregion

void EditorLayer::OnEvent(TS_ENGINE::Event& e)
{
	TS_ENGINE::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<TS_ENGINE::KeyPressedEvent>(TS_BIND_EVENT_FN(EditorLayer::OnKey, true));
	dispatcher.Dispatch<TS_ENGINE::KeyPressedEvent>(TS_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
	dispatcher.Dispatch<TS_ENGINE::KeyReleasedEvent>(TS_BIND_EVENT_FN(EditorLayer::OnKeyReleased));
	dispatcher.Dispatch<TS_ENGINE::MouseButtonPressedEvent>(TS_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
}

bool EditorLayer::OnKey(TS_ENGINE::KeyPressedEvent& e)
{
	switch (e.GetKeyCode())
	{
	case TS_ENGINE::Key::LeftControl:

		break;
	}
	return false;
}

bool EditorLayer::OnKeyPressed(TS_ENGINE::KeyPressedEvent& e)
{
	switch (e.GetKeyCode())
	{
	case TS_ENGINE::Key::Escape:
		TS_ENGINE::Application::GetInstance().Close();
		break;
	case TS_ENGINE::Key::Tab:	
		TS_ENGINE::Application::GetInstance().ToggleWireframeMode();
		break;
	case TS_ENGINE::Key::D1:
		//TS_ENGINE::Application::Get().EnableDiffuseMode();		
		break;
	case TS_ENGINE::Key::D2:
		TS_ENGINE::Application::GetInstance().ToggleTextures();
		break;
	case TS_ENGINE::Key::D3:
		//TS_ENGINE::Application::Get().EnableSpecularMode();	
		break;
	case TS_ENGINE::Key::G:
		//SpawnNewObject();
		break;
	case TS_ENGINE::Key::Delete:
		mSceneGui->DeleteSelectedNode();
		break;
	case TS_ENGINE::Key::LeftControl:
		mIsControlPressed = true;
		break;
	case TS_ENGINE::Key::D:
		if (mIsControlPressed)
		{
			mSceneGui->DuplicatedSelectedNode();
		}
		break;
	case TS_ENGINE::Key::N:
		if (mIsControlPressed)
		{
			if(!mSceneGui->m_ShowNewSceneWindow)
				mSceneGui->m_ShowNewSceneWindow = true;			
		}
		break;
	case TS_ENGINE::Key::O:
		if (mIsControlPressed)
		{
			
		}
		break;
	case TS_ENGINE::Key::S:
		if (mIsControlPressed)
		{
			if (TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene())
			{
				TS_ENGINE::SceneManager::GetInstance()->SaveCurrentScene();
				std::string sceneName = TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode()->GetEntity()->GetName();
				mSceneGui->TakeSnapshot(TS_ENGINE::Application::s_ThumbnailsDir.string() + "\\" + sceneName + ".png");
			}
			else
			{
				TS_CORE_ERROR("Current scene is not set!");
			}
		}
		break;
	case TS_ENGINE::Key::Q:
		mSceneGui->SwitchToTranslateMode();
		break;
	case TS_ENGINE::Key::E:
		mSceneGui->SwitchToRotateMode();
		break;
	case TS_ENGINE::Key::R:
		mSceneGui->SwitchToScaleMode();
		break;
	}

	return false;
}

bool EditorLayer::OnKeyReleased(TS_ENGINE::KeyReleasedEvent& e)
{
	switch (e.GetKeyCode())
	{
	case TS_ENGINE::Key::LeftControl:
		mIsControlPressed = false;
		break;
	}
	return false;
}

bool EditorLayer::OnMouseButtonPressed(TS_ENGINE::MouseButtonPressedEvent& e)
{
	if (e.GetMouseButton() == TS_ENGINE::Mouse::ButtonLeft)//Why is this creating an issue
	{
		if (!ImGuizmo::IsOver())
			mSceneGui->SetSelectedNode(nullptr);
	}

	return false;
}

void EditorLayer::OnOverlayRender()
{
	//This should render only for editor camera framebuffer
	if (TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene())
	{
		TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->ShowSceneCameraGUI(mCurrentShader, mDeltaTime);
	}
}

//Vector2 GetGridPosFromIndex(size_t index, size_t width)
//{
//	size_t x = index % width;
//	size_t y = (index - x) / width;
//
//	return Vector2(x, y);
//}

//void EditorLayer::SpawnNewObject()
//{
//	int randomIndex = 0 + (std::rand() % (6 - 0 + 1));
//	int randomColorIndex = 0 + (std::rand() % (6 - 0 + 1));
//
//	Ref<TS_ENGINE::Node> node = SpawnGameObjectNode(randomIndex, randomColorIndex);
//
//	//go->SetColor(ColorPallete[randomColorIndex]);
//	Vector2 gridPos = GetGridPosFromIndex(mNodes.size(), mGridSizeX);
//
//	if (randomIndex == 5 || randomIndex == 6)
//		node->GetTransform()->SetLocalPosition(2 * gridPos.x, -node->GetTransform()->GetLocalScale().x * 0.5f, 2 * gridPos.y);
//	else
//		node->GetTransform()->SetLocalPosition(2 * gridPos.x, 0, 2 * gridPos.y);
//
//	mScene1->GetSceneNode()->AddChild(mScene1->GetSceneNode(), node);
//	node->SetParentNode(mScene1->GetSceneNode());
//
//	mNodes.push_back(node);
//}

//Ref<TS_ENGINE::Node> EditorLayer::SpawnGameObjectNode(uint32_t index, uint32_t randColorIndex)//Index is a random number between 0 to 6
//{
//	if (index == 0)
//	{
//		Ref<TS_ENGINE::Node> node = CreateRef<TS_ENGINE::Node>();
//		node->SetName("QuadNode");
//		node->GetTransform()->SetLocalEulerAngles(0.0f, 180.0f, 0.0f);
//
//		Ref<TS_ENGINE::Quad> quad = CreateRef<TS_ENGINE::Quad>("Quad");
//		quad->SetColor(ColorPallete[randColorIndex]);
//		quad->SetTexture("Crate.png");
//		quad->Create();
//
//		node->AttachGameObject(quad);
//		return node;
//	}
//	else if (index == 1)
//	{
//		Ref<TS_ENGINE::Node> node = CreateRef<TS_ENGINE::Node>();
//		node->SetName("QuadNode");
//		node->GetTransform()->SetLocalEulerAngles(0.0f, 180.0f, 0.0f);
//
//		Ref<TS_ENGINE::Quad> quad = CreateRef<TS_ENGINE::Quad>("Quad");
//		quad->SetColor(ColorPallete[randColorIndex]);
//		quad->SetTexture("CrateTex1.png");
//		quad->Create();
//
//
//		node->AttachGameObject(quad);
//		return node;
//	}
//	else if (index == 2)
//	{
//		Ref<TS_ENGINE::Node> node = CreateRef<TS_ENGINE::Node>();
//		node->SetName("QuadNode");
//		node->GetTransform()->SetLocalEulerAngles(0.0f, 180.0f, 0.0f);
//
//		Ref<TS_ENGINE::Quad> quad = CreateRef<TS_ENGINE::Quad>("Quad");
//		quad->SetColor(ColorPallete[randColorIndex]);
//		quad->SetTexture("Terrain.png");
//		quad->Create();
//
//		node->AttachGameObject(quad);
//		return node;
//	}
//	else if (index == 3)
//	{
//		Ref<TS_ENGINE::Node> node = CreateRef<TS_ENGINE::Node>();
//		node->SetName("CubeNode");
//		node->GetTransform()->SetLocalEulerAngles(0, 180.0f + 90.0f, 0);
//
//		Ref<TS_ENGINE::Cube> cube = CreateRef<TS_ENGINE::Cube>("Cube");
//		cube->SetColor(ColorPallete[randColorIndex]);
//		cube->SetTexture("Crate.png");
//		cube->Create();
//
//		node->AttachGameObject(cube);
//		return node;
//	}
//	else if (index == 4)
//	{
//		Ref<TS_ENGINE::Node> node = CreateRef<TS_ENGINE::Node>();
//		node->SetName("CubeNode");
//		node->GetTransform()->SetLocalEulerAngles(0.0f, 180.0f + 45.0f, 0.0f);
//
//		Ref<TS_ENGINE::Cube> cube = CreateRef<TS_ENGINE::Cube>("Cube");
//		cube->SetColor(ColorPallete[randColorIndex]);
//		cube->SetTexture("Crate.png");
//		cube->Create();
//
//		node->AttachGameObject(cube);
//		return node;
//	}
//	else if (index == 5)
//	{
//		Ref<TS_ENGINE::Node> node = CreateRef<TS_ENGINE::Node>();
//		node->SetName("Model");
//		node->GetTransform()->SetLocalEulerAngles(-90, 0, 180.0f);
//		node->GetTransform()->SetLocalScale(1.0f, 1.0f, 1.0f);
//
//		mModelLoader->LoadModel("Assets\\Models", "monk_character.glb");
//		mModelLoader->GetLastLoadedModel()->SetName("Model1");
//
//		node->AttachGameObject(mModelLoader->GetLastLoadedModel());
//		return node;
//	}
//	else if (index == 6)
//	{
//		Ref<TS_ENGINE::Node> node = CreateRef<TS_ENGINE::Node>();
//		node->SetName("Model");
//		node->GetTransform()->SetLocalEulerAngles(-90, 0, 180.0f);
//		node->GetTransform()->SetLocalScale(1.0f, 1.0f, 1.0f);
//
//		mModelLoader->LoadModel("Assets\\Models", "monk_character.glb");
//		mModelLoader->GetLastLoadedModel()->SetName("Model2");
//
//		node->AttachGameObject(mModelLoader->GetLastLoadedModel());
//		return node;
//	}
//	else
//		return nullptr;
//}
