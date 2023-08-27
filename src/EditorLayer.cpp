#include "EditorLayer.h"
//#include <imgui_demo.cpp>
//#include <Factory.h>
#include <Core/Log.h>

#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <Core/Application.h>

#include <Core/Input.h>
#include <Core/KeyCodes.h>
#include <Utils/Utility.h>
#include <Platform/OpenGL/OpenGLVertexArray.h>

EditorLayer::EditorLayer() :
	Layer("SandboxLayer")//,
	//mBatchedGameObject(NULL)
{
	
}

void EditorLayer::OnAttach()
{
	mSceneGui = CreateRef<TS_ENGINE::SceneGui>();

#pragma region Shader
	//mDefaultShader = TS_ENGINE::Shader::Create("Lit", "Lit.vert", "Lit.frag");
	mDefaultShader = TS_ENGINE::Shader::Create("HDRLighting", "HDRLighting.vert", "HDRLighting.frag");
	//mBatchLitShader = TS_ENGINE::Shader::Create("BatchLit", "BatchLit.vert", "BatchLit.frag");
	//mHdrLightingShader = TS_ENGINE::Shader::Create("HDRLighting", "HDRLighting.vert", "HDRLighting.frag");

	//Material
	mDefaultMat = CreateRef<TS_ENGINE::Material>("DefaultMaterial", mDefaultShader);//Create default material
	//mHdrMat = CreateRef<TS_ENGINE::Material>("HdrLighting", mHdrLightingShader);//Create HDR material

	//Activate Shader
	mCurrentShader = mDefaultShader;
	mCurrentShader->Bind();
#pragma endregion

#pragma region Cameras
	mEditorCamera = CreateRef<TS_ENGINE::EditorCamera>("EditorCamera");

	mAspectRatio = (float)TS_ENGINE::Application::Get().GetWindow().GetWidth() / (float)TS_ENGINE::Application::Get().GetWindow().GetHeight();
	mEditorCamera->SetPerspective(TS_ENGINE::Camera::Perspective(45.0f, mAspectRatio, 0.1f, 1000.0f));
	mEditorCamera->GetNode()->GetTransform()->SetLocalPosition(-0.738f, 5.788f, 14.731f);
	mEditorCamera->GetNode()->GetTransform()->SetLocalEulerAngles(-18.102f, 0.066f, 0.0f);
	mEditorCamera->CreateFramebuffer(1920, 1080);//Create framebuffer for editorCamera
	mEditorCamera->Initialize();

	//Create and set current scene in SceneManager
	mScene1 = CreateRef<TS_ENGINE::Scene>("Scene1", mEditorCamera);	
	TS_ENGINE::SceneManager::GetInstance()->SetCurrentScene(mScene1);
}

void EditorLayer::OnDetach()
{
	mScene1.reset();
}

void EditorLayer::OnUpdate(float deltaTime)
{
	mDeltaTime = deltaTime;
	TS_ENGINE::Application::Get().ResetStats();

	mScene1->Render(mCurrentShader, deltaTime);

	//Editor camera pass
	{
		mScene1->UpdateCameraRT(mEditorCamera, mCurrentShader, deltaTime, true);

		//Render after all gameObjects rendered to show as an overlay.
		OnOverlayRender();
		
		if (!ImGuizmo::IsOver())
			PickGameObject();

		mEditorCamera->GetFramebuffer()->Unbind();
	}
}

void EditorLayer::PickNode(TS_ENGINE::Node* node, int entityID)
{
	if (node->GetMeshes().size() > 0)
	{
		//TS_CORE_TRACE("Checking: {0}", node->GetName());

		if (node->GetEntity()->GetEntityID() == entityID)
		{
			TS_CORE_TRACE("{0} has matching entityID", node->GetName());
			mMatchingNode = node;
		}
	}

	for (auto& childNode : node->GetChildren())
	{
		PickNode(childNode, entityID);
	}
}

TS_ENGINE::Node* EditorLayer::PickNodeByEntityID(int entityID)
{
	PickNode(mScene1->GetSceneNode(), entityID);
	return mMatchingNode;
}

void EditorLayer::PickGameObject()
{
	if (TS_ENGINE::Input::IsMouseButtonPressed(TS_ENGINE::Mouse::Button0) && !mMouseClicked)
	{
		//Picking code
		auto [mx, my] = ImGui::GetMousePos();

		mx -= mSceneGui->GetViewportBounds()[0].x;
		my -= mSceneGui->GetViewportBounds()[0].y;

		glm::vec2 viewportSize = mSceneGui->GetViewportBounds()[1] - mSceneGui->GetViewportBounds()[0];

		my = viewportSize.y - my;

		int mouseX = (int)mx - 8;//TODO: Offset (-8, 26) is needed for proper picking. Need to find the root cause of this issue.
		int mouseY = (int)my + 26;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			//Vector4 pixelColor = mEditorCamera->GetFramebuffer()->ReadPixelColor(0, mouseX, mouseY);
			//mPickedColor = ImVec4(pixelColor.x / 255.0f, pixelColor.y / 255.0f, pixelColor.z / 255.0f, pixelColor.z / 255.0f);
			//TS_CORE_TRACE("Pixel color = {0}", glm::to_string(pixelColor));

			int entityID = mEditorCamera->GetFramebuffer()->ReadPixel(1, mouseX, mouseY);
			TS_CORE_TRACE("Picking entity with ID : {0}", entityID);

			if (entityID != -1)
			{
				TS_ENGINE::Node* hoveredOnNode = nullptr;

				if (entityID != mScene1->GetSkyboxEntityID())// Avoid skybox selection
				{
					//Check if scene camera's GUI was selected
					if (mScene1->GetCurrentSceneCamera()->IsSceneCameraGuiSelected(entityID))
						hoveredOnNode = mScene1->GetCurrentSceneCamera()->GetNode().get();
					else
						hoveredOnNode = PickNodeByEntityID(entityID);

					if (hoveredOnNode != nullptr)
					{
						mSceneGui->SetSelectedNode(hoveredOnNode);
					}
				}
				else
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
	style.WindowBorderSize = 0.0f;

	defaultWindowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

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
			if (ImGui::MenuItem("Open", "CTRL+O"))
			{

			}
			if (ImGui::MenuItem("Save", "CTRL+S"))
			{
				TS_ENGINE::SceneManager::GetInstance()->SaveCurrentScene();
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

		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::BeginMenu("Primitive"))
			{
				if (ImGui::MenuItem("Quad"))
				{
					//TS_ENGINE::Factory::GetInstance()->CreateGameObject(TS_ENGINE::PrimitiveType::QUAD);
				}

				if (ImGui::MenuItem("Cube"))
				{
					//TS_ENGINE::Factory::GetInstance()->CreateGameObject(TS_ENGINE::PrimitiveType::CUBE);
				}

				if (ImGui::MenuItem("Sphere"))
				{
					//TS_ENGINE::Factory::GetInstance()->CreateGameObject(TS_ENGINE::PrimitiveType::SPHERE);
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

		ImGui::EndMainMenuBar();
	}
}

void EditorLayer::ShowSubMenu()
{
	//No need to set MainMenuBar size and position, DefaultSize: CurrentWindowWidth, 18.0f, DefaultPos: 0, 0
	ImVec2 subMenuPos = ImVec2(0.0f, 19.8f);
	ImVec2 subMenuSize = ImVec2((float)TS_ENGINE::Application::Get().GetWindow().GetWidth() - 250.0f, 34.5f);

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
	ImVec2 statsPanelPos = ImVec2(TS_ENGINE::Application::Get().GetWindow().GetWidth() - 450.0f, 19.0f);
	ImVec2 statsPanelSize = ImVec2(200.0f, 150.0f);
#pragma endregion

		mSceneGui->ShowViewportWindow(mEditorCamera, mScene1->GetCurrentSceneCamera());
		mSceneGui->ShowInspectorWindow();
		mSceneGui->ShowHierarchyWindow(mScene1);
		mSceneGui->ShowContentBrowser();
}

#pragma endregion

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
		//SpawnNewObject();
		break;
	case TS_ENGINE::Key::Delete:
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
	mScene1->GetCurrentSceneCamera()->RenderGui(mCurrentShader, mDeltaTime);//Render Scene camera's GUI
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
