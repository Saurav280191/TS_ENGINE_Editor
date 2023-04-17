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
#include <Renderer/Texture.h>
#include "Renderer/RenderCommand.h"
#include <SceneManager/Node.h>
#include <SceneManager/Scene.h>

#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#define IMAPP_IMPL
#include "ImGuizmo.h"

class EditorLayer : public TS_ENGINE::Layer
{
public:
	EditorLayer();
	virtual ~EditorLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(float deltaTime) override;

	virtual void OnImGUIRender() override;
	virtual void OnEvent(TS_ENGINE::Event& e) override;

	void SpawnNewObject();
	void CreateUIForAllNodes(Ref<TS_ENGINE::Node> node);
private:
	bool OnKeyPressed(TS_ENGINE::KeyPressedEvent& e);
	bool OnMouseButtonPressed(TS_ENGINE::MouseButtonPressedEvent& e);

	void OnOverlay();

	Ref<TS_ENGINE::Camera> mEditorCamera;

	bool mIsCurrentMaterialLit = false;

	Ref<TS_ENGINE::Shader> mLitShader;
	Ref<TS_ENGINE::Shader> mBatchLitShader;
	Ref<TS_ENGINE::Shader> mCurrentShader;

	std::vector<Ref<TS_ENGINE::Node>> mNodes;

	TS_ENGINE::GameObject* mBatchedGameObject;
	bool mBatched = false;

	uint32_t mGridSizeX = 5;
	uint32_t mGridSizeY = 5;

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
	Ref<TS_ENGINE::Node> SpawnGameObjectNode(uint32_t index, uint32_t randColorIndex);

	bool mOrthographicProjectionActive;

	Ref<TS_ENGINE::Scene> mCurrentScene;
	Ref<TS_ENGINE::Node> mSelectedNode;


	//For IMGUI
	float mCurrScale[3];
	float mLastScale[3];
	bool mShowStats = false;

	//For IMGUI	
	bool m_ScaleLock = false;

	Ref<TS_ENGINE::Texture2D> mUnlockedIcon, mLockedIcon,
		mMeshFilterIcon, mMeshRendererIcon,
		mMaterialIcon, mLitMaterialIcon;

	//ImGuizmo params
	ImGuizmo::OPERATION mTransformOperation = ImGuizmo::OPERATION::TRANSLATE;
	ImGuizmo::MODE mTransformMode = ImGuizmo::MODE::WORLD;
	bool mTranslateActive = true;
	bool mRotateActive = false;
	bool mScaleActive = false;
};

