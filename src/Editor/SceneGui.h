#pragma once
#include "SceneManager/Node.h"
#include "SceneManager/Scene.h"
#include <Renderer/Texture.h>
#include <Application.h>
#include <Utils/Utility.h>
#include <Utils/MyMath.h>
//#include <Factory.h>
#include <filesystem>
#include <json/include/nlohmann/json.hpp>

namespace TS_ENGINE {


	// Struct to store sprite rect data from json for GUI
	struct SpriteRect 
	{
		int x;
		int y;
		int width;
		int height;
	};

	struct NormalizedRect
	{
		ImVec2 topLeft;
		ImVec2 bottomRight;
		ImVec2 size;
	};

	class SceneGui
	{
	public:		
		enum ItemType
		{
			TEXTURE,
			MODEL,
			MATERIAL,
			SCENE
		};		

		SceneGui();
		~SceneGui();

		void ShowTransformGizmos(const float* view, const float* projection);

		void ShowViewportWindow();
		void ShowInspectorWindow();
		void ShowHierarchyWindow();
		void ShowContentBrowser();		
		void ShowAnimationPanel();
		void ShowNewSceneWindow();		

		void TakeSnapshot(const std::string& snapshotPath);

		Ref<Node> GetSelectedNode() { return mSelectedNode; }

		void SwitchToTranslateMode();
		void SwitchToRotateMode();
		void SwitchToScaleMode();

		ImGuizmo::OPERATION GetTransformOperation() { return mTransformOperation; }
		ImGuizmo::MODE GetTransformMode() { return mTransformMode; }

		void SetSelectedNode(Ref<Node> node);

		ImVec2 GetViewportPos() { return mViewportPos; }

		ImVec2 GetViewportSize() { return mViewportSize; }

		Ref<Rect> GetViewportImageRect() { return mViewportImageRect; }

		void DeleteSelectedNode();
		void DuplicatedSelectedNode();
	public:
		Vector3 mSelectedNodeLocalPosition = Vector3(0, 0, 0);
		Vector3 mSelectedNodeLocalEulerAngles = Vector3(0, 0, 0);
		Vector3 mSelectedNodeLocalScale = Vector3(1, 1, 1);
		
		bool IsViewportActiveWindow = false;
		bool m_ShowNewSceneWindow = false;
	private:	
		void CreateUIForAllNodes(Ref<Node> node);

		void DragHierarchySceneNode(Ref<Node> node);
		void DragContentBrowserItem(const char* filePath, ItemType itemType);

		void DropHierarchySceneNode(Ref<Node> targetParentNode);		
		void DropItemInViewport();
		
		void CaptureSnapshot(const Ref<Framebuffer>& _framebuffer, const std::string& _filepath);

		// Shows FPS, DrawCalls, Vertices, Indices etc.
		void ShowStatsWindow(ImVec2& _statsPanelPos, ImVec2& _statsPanelSize, bool& _statsWindowExpanded);
		// Shows Wireframe, ToggleTexture, BoneView, & BoneInfluence buttons
		void ShowRenderingModesButtons(ImVec2& _renderingModesWindowPos, ImVec2& _renderingModesWindowSize);
	private:
		uint64_t mEditorCameraRenderTextureID = 0;
		char mSelectedNodeNameBuffer[256] = "";
		char mNewSceneText[256] = "NewScene";

		Ref<Texture2D> mIconSpriteSheetTexture;
		std::unordered_map<std::string, NormalizedRect> mIconRectMap;
		NormalizedRect playButtonRect;

		std::unordered_map<std::string, Ref<Texture2D>> mSavedSceneThumbnails;
	
		Ref<Node> mSelectedNode = nullptr;
		Ref<Node> mHoveringOnNode = nullptr;
		bool mNodePopedUp = false;		

		//ImGuizmo params
		ImGuizmo::OPERATION mTransformOperation = ImGuizmo::OPERATION::TRANSLATE;
		ImGuizmo::MODE mTransformMode = ImGuizmo::MODE::LOCAL;

		bool mTranslateActive = true;
		bool mRotateActive = false;
		bool mScaleActive = false;
		bool mJustSelected = false;

		ImVec2 mViewportPos;
		ImVec2 mViewportSize;
		Ref<Rect> mViewportImageRect;

		bool mTakeSnap = false;
		std::string mSnapshotPath = "";

		const char* mTransformComboItems[2];// = new const char* [2] {"Local", "World"};
		const char* mMeshNameList[6] = {
			//"Line",//TODO: Line need to have it's own component like LineRenderer
			"Quad",
			"Cube",
			"Sphere",
			"Cylinder",
			"Cone",
			"Model"
		};
		const char* mCurrentMeshItem = "Default";
		
		const char* mProjectionList[2] = {
			"Perspective",
			"Orthographic"
		};
		const char* mCurrentProjection = "Default";

		int mTransformCurrentItem = 0;
		int mCurrentMeshIndex = 0;
		std::filesystem::path mCurrentDirectory;
	};
}
