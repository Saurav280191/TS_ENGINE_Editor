#pragma once
#include "SceneManager/Node.h"
#include "SceneManager/Scene.h"
#include <Renderer/Texture.h>
#include <GameObject.h>
#include <Application.h>
#include <Utils/Utility.h>
#include <Utils/MyMath.h>
#include <Factory.h>

namespace TS_ENGINE {

	class SceneGui
	{
	public:
		SceneGui();
		virtual ~SceneGui() = default;

		void ShowTransformGizmos(const float* view, const float* projection);

		void ShowViewportWindow(Ref<TS_ENGINE::Camera> editorCamera, Ref<TS_ENGINE::Camera> currentSceneCamera);
		void ShowStatsWindow(ImVec2 statsPanelPos, ImVec2 statsPanelSize);		
		void ShowInspectorWindow();
		void ShowHierarchyWindow(Ref<TS_ENGINE::Scene> scene);
		void ShowContentBrowser();

		Ref<Node> GetSelectedNode();

		void SwitchToTranslateMode();
		void SwitchToRotateMode();
		void SwitchToScaleMode();

		ImGuizmo::OPERATION GetTransformOperation()
		{
			return mTransformOperation;
		}

		ImGuizmo::MODE GetTransformMode()
		{
			return mTransformMode;
		}
		
		void SetSelectedNode(Ref<TS_ENGINE::Node> node);

		ImVec2 GetViewportPos()
		{
			return mViewportPos;
		}

		ImVec2 GetViewportSize()
		{
			return mViewportSize;
		}

		Vector2* GetViewportBounds()
		{
			return mViewportBounds;
		}

	private:
		void HandleNodeDragDrop(Ref<TS_ENGINE::Node> _pickedNode, Ref<TS_ENGINE::Node> _targetParentNode);
		void CreateUIForAllNodes(const Ref<TS_ENGINE::Node> node);

		Ref<Texture2D> mMeshEditorIcon;
		Ref<Texture2D> mMaterialEditorIcon;
		Ref<Node> mSelectedNode;

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
		const char* mMeshNameList[7] = {
			"Quad",
			"Cube",
			"Sphere",
			"Cone",
			"Cylinder",
			"Model",
			"Empty"
		};
		
		const char* mCurrentMeshItem = "Default";
		
		int mTransformCurrentItem = 0;
		int mCurrentMeshIndex = 0;
	public:
		Vector3 mSelectedNodePosition;
		Vector3 mSelectedNodeEulerAngles;
		Vector3 mSelectedNodeScale;		
	};
}
