#pragma once
#include "SceneManager/Node.h"
#include "SceneManager/Scene.h"
#include <Renderer/Texture.h>
#include <Application.h>
#include <Utils/Utility.h>
#include <Utils/MyMath.h>
//#include <Factory.h>
#include <filesystem>

namespace TS_ENGINE {

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
		virtual ~SceneGui() = default;

		void ShowTransformGizmos(const float* view, const float* projection);

		void ShowViewportWindow();
		void ShowStatsWindow(ImVec2 statsPanelPos, ImVec2 statsPanelSize);
		void ShowInspectorWindow();
		void ShowHierarchyWindow();
		void ShowContentBrowser();
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
		
		void CaptureSnapshot(Ref<Rect> rect, std::string path);
	
	private:
		uint64_t mEditorCameraRenderTextureID = 0;
		char mSelectedNodeNameBuffer[256] = "";
		char mNewSceneText[256] = "NewScene";

		Ref<Texture2D> mMeshEditorIcon;
		Ref<Texture2D> mCameraIcon;
		Ref<Texture2D> mMaterialEditorIcon;
		Ref<Texture2D> mContentBrowserDirectoryIcon;
		Ref<Texture2D> mContentBrowserModelFileIcon;
		Ref<Texture2D> mContentBrowserImageFileIcon;
		Ref<Texture2D> mContentBrowserShaderFileIcon;
		Ref<Texture2D> mContentBrowserMiscFileIcon;
		Ref<Texture2D> mSceneFileIcon;
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
