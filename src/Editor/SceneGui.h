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

		void ShowViewportWindow(ImVec2 viewportPanelPos, ImVec2 viewportPanelSize, 
			Ref<TS_ENGINE::Camera> mEditorCamera, Ref<TS_ENGINE::Camera> mSceneCamera);

		void ShowStatsWindow(ImVec2 statsPanelPos, ImVec2 statsPanelSize);		
		void ShowInspectorWindow(ImVec2 inspectorPanelPos, ImVec2 inspectorPanelSize);
		void ShowHierarchyWindow(Ref<TS_ENGINE::Scene> scene, ImVec2 hierarchyPanelPos, ImVec2 hierarchyPanelSize);

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

	private:
		void HandleNodeDragDrop(Ref<TS_ENGINE::Node> _pickedNode, Ref<TS_ENGINE::Node> _targetParentNode);
		void CreateUIForAllNodes(const Ref<TS_ENGINE::Node> node);

		Ref<Texture2D> mMeshEditorIcon;
		Ref<Texture2D> mMaterialEditorIcon;
		Ref<Node> mSelectedNode;

		//ImGuizmo params
		ImGuizmo::OPERATION mTransformOperation = ImGuizmo::OPERATION::TRANSLATE;
		
		ImGuizmo::MODE mTransformMode = ImGuizmo::MODE::LOCAL;		
		const char* mTransformComboItems[2];// = new const char* [2] {"Local", "World"};
		int mTransformCurrentItem = 0;

		bool mTranslateActive = true;
		bool mRotateActive = false;
		bool mScaleActive = false;
		bool mJustSelected = false;

		const char* mCurrentMeshItem = "Default";
		int mCurrentMeshIndex;
		const char* mMeshNameList[7] = {
			"Quad",
			"Cube",
			"Sphere",
			"Cone",
			"Cylinder",
			"Model",
			"Empty"
		};
	public:
		Vector3 mSelectedNodePosition;
		Vector3 mSelectedNodeEulerAngles;
		Vector3 mSelectedNodeScale;
	};
}
