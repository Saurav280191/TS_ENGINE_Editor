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
//#include <Factory.h>

//Scene management headers
#include <SceneManager/Node.h>
#include <SceneManager/Scene.h>
//#include <SceneManager/SceneSerializer.h>
#include <SceneManager/SceneManager.h>

//Camera headers
#include <Renderer/Camera/EditorCamera.h>
#include "Editor/SceneGui.h"


#include <imgui.h>
//#define IMGUI_DEFINE_MATH_OPERATORS // Already set in preprocessors
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
private:
	bool OnKey(TS_ENGINE::KeyPressedEvent& e);
	bool OnKeyPressed(TS_ENGINE::KeyPressedEvent& e);
	bool OnKeyReleased(TS_ENGINE::KeyReleasedEvent& e);
	bool OnMouseButtonPressed(TS_ENGINE::MouseButtonPressedEvent& e);
	void OnOverlayRender();	
	void PickGameObject();
	Ref<TS_ENGINE::Node> PickNodeByEntityID(int entityID);
	void PickNode(Ref<TS_ENGINE::Node> node, int entityID);
private:
	bool mIsControlPressed = false;
	ImGuiWindowFlags defaultWindowFlags;

	Ref<TS_ENGINE::SceneGui> mSceneGui;
	//float mAspectRatio;
	//Ref<TS_ENGINE::EditorCamera> mEditorCamera;
	
	ImVec2 mViewportPanelPos;
	ImVec2 mViewportPanelSize;
	
	bool mMouseClicked = false;
	Ref<TS_ENGINE::Node> mMatchingNode = nullptr;

	Ref<TS_ENGINE::Shader> mCurrentShader;
	Ref<TS_ENGINE::Shader> mDefaultMaterial;
	bool mIsCurrentMaterialLit = false;

	float mDeltaTime;

	bool mOrthographicProjectionActive;	
	
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

