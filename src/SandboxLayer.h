#pragma once
#include "TS_ENGINE.h"
#include <Events/KeyEvent.h>
#include <Events/MouseEvent.h>
#include <Renderer/Camera.h>
#include <Renderer/Texture.h>
#include <Primitive/Quad.h>
#include <Primitive/Cube.h>
#include <Primitive/Sphere.h>
#include <ModelLoader.h>
#include <Renderer/Batcher.h>
#include <Renderer/Texture.h>
#include "Renderer/RenderCommand.h"

class BatchButton
{
public:
	typedef std::function<void(Ref<TS_ENGINE::Shader>& currentShader, std::vector<Ref<TS_ENGINE::GameObject>> gameObjects)> ClickHandler;

	void RegisterClickHandler(ClickHandler handler)
	{
		mClickHandler = handler;
	}

	void Click(Ref<TS_ENGINE::Shader>& currentShader, std::vector<Ref<TS_ENGINE::GameObject>> gameObjects)
	{
		mClickHandler(currentShader, gameObjects);
	}

private:
	ClickHandler mClickHandler;
};

class ButtonHandler
{
public:
	void OnButtonClicked(const Ref<TS_ENGINE::Shader>& currentShader, std::vector<Ref<TS_ENGINE::GameObject>> gameObjects)
	{
		currentShader->Bind();

		if (!TS_ENGINE::Batcher::GetInstance()->IsBatched())
		{
			if (gameObjects.size() == 0)
			{
				TS_CORE_WARN("No gameObjects to batch!");
				return;
			}

			TS_CORE_INFO("***Started batching gameobjects***");

			//Reset stats
			TS_ENGINE::Application::Get().ResetStats();

			if (gameObjects.size() > 0)
			{
				TS_ENGINE::Batcher::GetInstance()->CreateAtlases();
				TS_ENGINE::Batcher::GetInstance()->StartBatching(gameObjects);
			}
		}
	}
};

class SandboxLayer : public TS_ENGINE::Layer
{
public:
	SandboxLayer();
	virtual ~SandboxLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(float deltaTime) override;

	virtual void OnImGUIRender() override;
	virtual void OnEvent(TS_ENGINE::Event& e) override;

	void SpawnNewObject();
private:
	bool OnKeyPressed(TS_ENGINE::KeyPressedEvent& e);
	bool OnMouseButtonPressed(TS_ENGINE::MouseButtonPressedEvent& e);

	void OnOverlay();

	TS_ENGINE::Camera* mSceneCamera;

	bool mIsCurrentMaterialLit = false;

	bool mTranslateActive = true;
	bool mRotateActive = false;
	bool mScaleActive = false;

	Ref<TS_ENGINE::Shader> mLitShader;
	Ref<TS_ENGINE::Shader> mBatchLitShader;
	Ref<TS_ENGINE::Shader> mCurrentShader;

	std::vector<Ref<TS_ENGINE::GameObject>> mGameObjects;

	TS_ENGINE::GameObject* mBatchedGameObject;
	bool mBatched = false;

	uint32_t mGridSizeX = 50;
	uint32_t mGridSizeY = 50;

	Vector3 ColorPallete[7] =
	{
		Vector3(148 , 0, 211) * Vector3(0.003921568627451f),
		Vector3(75, 0, 130) * Vector3(0.003921568627451f),
		Vector3(0, 0, 255) * Vector3(0.003921568627451f),
		Vector3(0, 255, 0) * Vector3(0.003921568627451f),
		Vector3(255, 255, 0) * Vector3(0.003921568627451f),
		Vector3(255, 127, 0) * Vector3(0.003921568627451f),
		Vector3(255, 0, 0) * Vector3(0.003921568627451f)
	};

	Ref<TS_ENGINE::ModelLoader> mModelLoader;
	Ref<TS_ENGINE::GameObject> SpawnGameObject(uint32_t index, uint32_t randColorIndex);

	bool mOrthographicProjectionActive;

	bool mBatchingEnabled;
	BatchButton mBatchButton;
	ButtonHandler mBatchButtonHandler;
};

