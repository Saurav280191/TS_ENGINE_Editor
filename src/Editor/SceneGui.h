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
			MATERIAL
		};		

		SceneGui();
		virtual ~SceneGui() = default;

		void ShowTransformGizmos(const float* view, const float* projection);

		void ShowViewportWindow(Ref<TS_ENGINE::Camera> editorCamera, Ref<TS_ENGINE::Camera> currentSceneCamera);
		void ShowStatsWindow(ImVec2 statsPanelPos, ImVec2 statsPanelSize);
		void ShowInspectorWindow();
		void ShowHierarchyWindow(Ref<TS_ENGINE::Scene> scene);
		void ShowContentBrowser();

		Ref<Node> GetSelectedNode() { return mSelectedNode; }

		void SwitchToTranslateMode();
		void SwitchToRotateMode();
		void SwitchToScaleMode();

		ImGuizmo::OPERATION GetTransformOperation() { return mTransformOperation; }
		ImGuizmo::MODE GetTransformMode() { return mTransformMode; }

		void SetSelectedNode(Ref<Node> node);

		ImVec2 GetViewportPos() { return mViewportPos; }

		ImVec2 GetViewportSize() { return mViewportSize; }

		Vector2* GetViewportBounds() { return mViewportBounds; }

	public:
		Vector3 mSelectedNodeLocalPosition = Vector3(0, 0, 0);
		Vector3 mSelectedNodeLocalEulerAngles = Vector3(0, 0, 0);
		Vector3 mSelectedNodeLocalScale = Vector3(1, 1, 1);
		
		bool IsViewportActiveWindow = false;

	private:	
		void CreateUIForAllNodes(int& nodeTreeGuiIndex, Ref<Node> node);

		void DragHierarchySceneNode(Ref<Node> node);
		void DragContentBrowserItem(const char* filePath, ItemType itemType);

		void DropHierarchySceneNode(Ref<Node> targetParentNode);		
		void DropItemInViewport();
		
		char* mSelectedNodeNameBuffer = new char[256];	

		Ref<Texture2D> mMeshEditorIcon;
		Ref<Texture2D> mMaterialEditorIcon;
		Ref<Texture2D> mContentBrowserDirectoryIcon;
		Ref<Texture2D> mContentBrowserModelFileIcon;
		Ref<Texture2D> mContentBrowserImageFileIcon;
		Ref<Texture2D> mContentBrowserShaderFileIcon;
		Ref<Texture2D> mContentBrowserMiscFileIcon;
	
	private:
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
		Vector2 mViewportBounds[2];

		const char* mTransformComboItems[2];// = new const char* [2] {"Local", "World"};
		const char* mMeshNameList[6] = {
			"Quad",
			"Cube",
			"Sphere",
			"Cone",
			"Cylinder",
			"Empty"
		};

		const char* mCurrentMeshItem = "Default";

		int mTransformCurrentItem = 0;
		int mCurrentMeshIndex = 0;
		std::filesystem::path mCurrentDirectory;
	};
}
