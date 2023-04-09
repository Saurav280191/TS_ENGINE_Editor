#include "SandboxLayer.h"
//#include "Renderer/Framebuffer.h"
//#include <imgui_demo.cpp>
//#include <Factory.h>
#include <Core/Log.h>

#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <Core/Application.h>

#include <Core/Input.h>
#include <Core/KeyCodes.h>

SandboxLayer::SandboxLayer() :
	Layer("SandboxLayer"),
	mBatchingEnabled(false),
	mBatchedGameObject(NULL)
{

}

void SandboxLayer::OnAttach()
{
	mSceneCamera = new TS_ENGINE::Camera(TS_ENGINE::Camera::SCENECAMERA);

	float aspectRatio = (float)TS_ENGINE::Application::Get().GetWindow().GetWidth() / (float)TS_ENGINE::Application::Get().GetWindow().GetHeight();

	TS_ENGINE::Camera::Perspective perspective;
	perspective.fov = 45.0f;
	perspective.aspectRatio = aspectRatio;
	perspective.zNear = 0.1f;
	perspective.zFar = 1000.0f;
	mSceneCamera->SetPerspective(perspective);

	mLitShader = TS_ENGINE::Shader::Create("Lit", "Lit.vert", "Lit.frag");
	mBatchLitShader = TS_ENGINE::Shader::Create("BatchLit", "BatchLit.vert", "BatchLit.frag");

	mModelLoader = CreateRef<TS_ENGINE::ModelLoader>();

	for (uint32_t i = 0; i < mGridSizeX * mGridSizeY; i++)
		SpawnNewObject();

	mSceneCamera->SetPosition(105.280922f, 10.1856785f, 105.097504f);//For 50x50 Grid
	mSceneCamera->SetEulerAngles(0.308291346f, -0.783849120f, 0.0f);

	//mSceneCamera->SetPosition(65.4178528f, 1.79701405f, 65.4272217f);//For 317x317 Grid
	//mSceneCamera->SetEulerAngles(0.0597731248f, -0.787812710f, 0.0f);

	mCurrentShader = mLitShader;
	mCurrentShader->Bind();	

	mBatchButton.RegisterClickHandler(std::bind(&ButtonHandler::OnButtonClicked, &mBatchButtonHandler, std::placeholders::_1, std::placeholders::_2));
}

Vector2 GetGridPosFromIndex(size_t index, size_t width)
{
	size_t x = index % width;
	size_t y = (index - x) / width;

	return Vector2(x, y);
}

void SandboxLayer::SpawnNewObject()
{
	int randomIndex = 0 + (std::rand() % (6 - 0 + 1));
	int randomColorIndex = 0 + (std::rand() % (6 - 0 + 1));

	auto go = SpawnGameObject(randomIndex, randomColorIndex);
	//go->SetColor(ColorPallete[randomColorIndex]);
	Vector2 gridPos = GetGridPosFromIndex(mGameObjects.size(), mGridSizeX);
	go->SetPosition(2 * gridPos.x, 0, 2 * gridPos.y);
	
	mGameObjects.push_back(go);
}

void SandboxLayer::OnDetach()
{
	if (mGameObjects.size() > 0)
		mGameObjects.clear();
}

void SandboxLayer::OnUpdate(float deltaTime)
{
	TS_ENGINE::Application::Get().ResetStats();

	mSceneCamera->OnUpdate(deltaTime);

	TS_ENGINE::RenderCommand::SetClearColor(Vector4(0.2f, 0.3f, 0.3f, 1.0f));
	TS_ENGINE::RenderCommand::Clear();

	mCurrentShader->SetMat4("u_View", mSceneCamera->GetViewMatrix());
	mCurrentShader->SetMat4("u_Projection", mSceneCamera->GetProjectionMatrix());

	if (mBatchingEnabled)
	{
		if (TS_ENGINE::Batcher::GetInstance()->IsBatched())
			TS_ENGINE::Batcher::GetInstance()->DrawBatch();
	}
	else
	{
		for (auto& go : mGameObjects)
			go->Draw(mCurrentShader);
	}
}

void SandboxLayer::OnImGUIRender()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	float minWinSizeX = style.WindowMinSize.x;
	style.WindowMinSize.x = 370.0f;

	style.WindowMinSize.x = minWinSizeX;


	ImGui::Begin("Stats");
	{
		ImGui::SetWindowPos(ImVec2(TS_ENGINE::Application::Get().GetWindow().GetWidth() - ImGui::GetWindowSize().x, 0));

		if (ImGui::Checkbox("Batching enabled", &mBatchingEnabled))
		{
			if (mBatchingEnabled)
				mCurrentShader = mBatchLitShader;
			else
				mCurrentShader = mLitShader;

			mBatchButton.Click(mCurrentShader, mGameObjects);
		}

		ImGui::Text("FPS: %.1f, %.3f ms/frame", 1000.0f / TS_ENGINE::Application::Get().GetDeltaTime(), TS_ENGINE::Application::Get().GetDeltaTime());

		ImGui::Text("Draw Calls: %d", TS_ENGINE::Application::Get().GetDrawCalls());
		ImGui::Text("Vertices: %d", TS_ENGINE::Application::Get().GetTotalVertices());
		ImGui::Text("Indices: %d", TS_ENGINE::Application::Get().GetTotalIndices());
	}
	ImGui::End();
}

void SandboxLayer::OnEvent(TS_ENGINE::Event& e)
{
	TS_ENGINE::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<TS_ENGINE::KeyPressedEvent>(TS_BIND_EVENT_FN(SandboxLayer::OnKeyPressed));
	dispatcher.Dispatch<TS_ENGINE::MouseButtonPressedEvent>(TS_BIND_EVENT_FN(SandboxLayer::OnMouseButtonPressed));
}

bool SandboxLayer::OnKeyPressed(TS_ENGINE::KeyPressedEvent& e)
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

bool SandboxLayer::OnMouseButtonPressed(TS_ENGINE::MouseButtonPressedEvent& e)
{
	return false;
}

void SandboxLayer::OnOverlay()
{

}

Ref<TS_ENGINE::GameObject> SandboxLayer::SpawnGameObject(uint32_t index, uint32_t randColorIndex)//Index is a random number between 0 to 6
{
	if (index == 0)
	{
		Ref<TS_ENGINE::Quad> quad = CreateRef<TS_ENGINE::Quad>("Quad");
		quad->SetColor(ColorPallete[randColorIndex]);
		quad->SetTexture("Crate.png");
		quad->SetEulerAngle(0.0f, 180.0f, 0.0f);
		quad->Create();
		return quad;
	}
	else if (index == 1)
	{
		Ref<TS_ENGINE::Quad> quad = CreateRef<TS_ENGINE::Quad>("Quad");
		quad->SetColor(ColorPallete[randColorIndex]);
		quad->SetTexture("CrateTex1.png");
		quad->SetEulerAngle(0.0f, 180.0f, 0.0f);
		quad->Create();
		return quad;
	}
	else if (index == 2)
	{
		Ref<TS_ENGINE::Quad> quad = CreateRef<TS_ENGINE::Quad>("Quad");
		quad->SetColor(ColorPallete[randColorIndex]);
		quad->SetTexture("Terrain.png");
		quad->SetEulerAngle(0.0f, 180.0f, 0.0f);
		quad->Create();
		return quad;
	}
	else if (index == 3) 
	{
		Ref<TS_ENGINE::Cube> cube = CreateRef<TS_ENGINE::Cube>("Cube");
		cube->SetColor(ColorPallete[randColorIndex]);
		cube->SetTexture("Crate.png");
		cube->SetEulerAngle(0, 180.0f + 90.0f, 0);
		cube->Create();
		return cube;
	}
	else if (index == 4)
	{
		Ref<TS_ENGINE::Cube> cube = CreateRef<TS_ENGINE::Cube>("Cube");
		cube->SetColor(ColorPallete[randColorIndex]);
		cube->SetTexture("Crate.png");
		cube->SetEulerAngle(0.0f, 180.0f + 45.0f, 0.0f);
		cube->Create();
		return cube;
	}
	else if (index == 5)
	{				
		mModelLoader->LoadModel("Assets\\Models", "monk_character.glb");
		mModelLoader->GetLastLoadedModel()->SetName("Model1");		
		mModelLoader->GetLastLoadedModel()->SetEulerAngle(-90, 0, 180.0f);
		mModelLoader->GetLastLoadedModel()->SetScale(0.5f);
		return mModelLoader->GetLastLoadedModel();
	}
	else if (index == 6)
	{
		mModelLoader->LoadModel("Assets\\Models", "monk_character.glb");
		mModelLoader->GetLastLoadedModel()->SetName("Model2");				
		mModelLoader->GetLastLoadedModel()->SetEulerAngle(-90, 0, 180.0f);
		mModelLoader->GetLastLoadedModel()->SetScale(0.5f);
		return mModelLoader->GetLastLoadedModel();
	}
	else
		return nullptr;
}
