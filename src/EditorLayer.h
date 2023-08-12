#pragma once
#include "TS_ENGINE.h"
#include <Events/KeyEvent.h>
#include <Events/MouseEvent.h>

//Renderer headers
#include <Renderer/Texture.h>
#include "Renderer/RenderCommand.h"
#include <Renderer/Camera/SceneCamera.h>
#include "Renderer/RendererAPI.h"
#include "Renderer/Framebuffer.h"
#include <Renderer/Material.h>
#include <Renderer/Texture.h>
#include <Renderer/Lighting/Light.h>

//Primitive headers
#include <Primitive/Quad.h>
#include <Primitive/Cube.h>
#include <Primitive/Sphere.h>
#include <Primitive/Line.h>

//Other object classes
#include <ModelLoader.h>
#include <Factory.h>

//Scene management headers
#include <SceneManager/Node.h>
#include <SceneManager/Scene.h>
#include <SceneManager/SceneSerializer.h>
#include <SceneManager/SceneManager.h>

//Camera headers
#include "Editor/EditorCamera.h"
#include "Editor/SceneGui.h"


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

#pragma region ImGUI functions
	virtual void OnImGuiRender() override;
	void ShowMainMenuBar();
	void ShowSubMenu();
	void ShowPanels();
#pragma endregion

	virtual void OnEvent(TS_ENGINE::Event& e) override;

	void PickGameObject();
	void UpdateCameraRT(Ref<TS_ENGINE::Camera> camera, float deltaTime, bool isEditorCamera = false);
private:
	bool OnKeyPressed(TS_ENGINE::KeyPressedEvent& e);
	bool OnMouseButtonPressed(TS_ENGINE::MouseButtonPressedEvent& e);
	void OnOverlayRender();

	ImGuiWindowFlags defaultWindowFlags;

	Ref<TS_ENGINE::Scene> mScene1;
	Ref<TS_ENGINE::SceneGui> mSceneGui;
	float mAspectRatio;
	
	Ref<EditorCamera> mEditorCamera;
	Ref<TS_ENGINE::SceneCamera> mCurrentSceneCamera;

	ImVec2 mViewportPanelPos;
	ImVec2 mViewportPanelSize;

	bool mMouseClicked = false;

	bool mIsCurrentMaterialLit = false;

	Ref<TS_ENGINE::Shader> mDefaultShader;
	Ref<TS_ENGINE::Shader> mBatchLitShader;
	Ref<TS_ENGINE::Shader> mHdrLightingShader;
	Ref<TS_ENGINE::Shader> mCurrentShader;

	float mDeltaTime;

	Ref<TS_ENGINE::Material> mDefaultMat;
	Ref<TS_ENGINE::Material> mHdrMat;

	Ref<TS_ENGINE::Light> mDirectionalLight;

	bool mOrthographicProjectionActive;

	//Ref<TS_ENGINE::Texture2D> 
		//mUnlockedIcon, 
		//mLockedIcon,
		//mMeshFilterIcon, 
		//mMeshRendererIcon,
		//mMaterialIcon, 
		//mLitMaterialIcon;
		

	//ImVec4 mPickedColor;
	//TS_ENGINE::GameObject* mBatchedGameObject;
	//bool mBatched = false;

	//bool mShowGrid = false;
	//bool mShowTranformGizmo = true;
	
	//Gamma and HDR
	//bool mGammaCorrection = false;
	//float mGammaValue = 1.0f;
	//bool mHdr = false;
	//float mHdrExposure = 1.1f;	

	//uint32_t mGridSizeX = 5;
	//uint32_t mGridSizeY = 5;

	//Ref<TS_ENGINE::ModelLoader> mModelLoader;
	//void SpawnNewObject();
	//Ref<TS_ENGINE::Node> SpawnGameObjectNode(uint32_t index, uint32_t randColorIndex);

	/*Vector3 ColorPallete[7] =
	{
		Vector3(148 , 0, 211) * Vector3(0.003921568627451f),
		Vector3(75, 0, 130) * Vector3(0.003921568627451f),
		Vector3(0, 0, 255) * Vector3(0.003921568627451f),
		Vector3(0, 255, 0) * Vector3(0.003921568627451f),
		Vector3(255, 255, 0) * Vector3(0.003921568627451f),
		Vector3(255, 127, 0) * Vector3(0.003921568627451f),
		Vector3(255, 0, 0) * Vector3(0.003921568627451f)
	};*/
};

