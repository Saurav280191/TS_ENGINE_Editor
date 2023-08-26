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
		SceneGui();
		virtual ~SceneGui() = default;

		void ShowTransformGizmos(const float* view, const float* projection);

		void ShowViewportWindow(Ref<TS_ENGINE::Camera> editorCamera, Ref<TS_ENGINE::Camera> currentSceneCamera);
		void ShowStatsWindow(ImVec2 statsPanelPos, ImVec2 statsPanelSize);
		void ShowInspectorWindow();
		void ShowHierarchyWindow(Ref<TS_ENGINE::Scene> scene);
		void ShowContentBrowser();

		Node* GetSelectedNode() { return mSelectedNode; }

		void SwitchToTranslateMode();
		void SwitchToRotateMode();
		void SwitchToScaleMode();

		ImGuizmo::OPERATION GetTransformOperation() { return mTransformOperation; }
		ImGuizmo::MODE GetTransformMode() { return mTransformMode; }

		void SetSelectedNode(TS_ENGINE::Node* node);

		ImVec2 GetViewportPos() { return mViewportPos; }

		ImVec2 GetViewportSize() { return mViewportSize; }

		Vector2* GetViewportBounds() { return mViewportBounds; }

	private:
		enum ItemType
		{
			TEXTURE,
			MODEL,
			MATERIAL
		};
		enum TextureType
		{
			DIFFUSE,
			SPECULAR,
			NORMAL
		};
		//Material properties
		struct MaterialGui
		{
			Vector4 mAmbientColor = Vector4(1, 1, 1, 1);
			Vector4 mDiffuseColor = Vector4(1, 1, 1, 1);
			Ref<Texture2D> mDiffuseMap = nullptr;
			float testFloat = 0.0f;
			float testFloat1 = 0.0f;
			float* mDiffuseMapOffset = nullptr;
			float* mDiffuseMapTiling = nullptr;
			Vector4 mSpecularColor = Vector4(1, 1, 1, 1);
			Ref<Texture2D> mSpecularMap = nullptr;
			float* mSpecularMapOffset = nullptr;
			float* mSpecularMapTiling = nullptr;
			float mShininess = 0.0f;
			Ref<Texture2D> mNormalMap = nullptr;
			float* mNormalMapOffset = nullptr;
			float* mNormalMapTiling = nullptr;
			float mBumpValue = 0.0f;
		};

		void ShowAllMaterials();
		
		void CreateUIForAllNodes(int& nodeTreeGuiIndex, TS_ENGINE::Node* node);

		void DragHierarchySceneNode(Node* node);
		void DragContentBrowserItem(const char* filePath, ItemType itemType);

		void DropHierarchySceneNode(Node* targetParentNode);
		void DropContentBrowserTexture(TextureType textureType, MaterialGui& materialGui, int meshIndex);
		void DropItemInViewport();


		Ref<Texture2D> mMeshEditorIcon;
		Ref<Texture2D> mMaterialEditorIcon;
		Ref<Texture2D> mContentBrowserDirectoryIcon;
		Ref<Texture2D> mContentBrowserModelFileIcon;
		Ref<Texture2D> mContentBrowserImageFileIcon;
		Ref<Texture2D> mContentBrowserShaderFileIcon;
		Ref<Texture2D> mContentBrowserMiscFileIcon;

		Node* mSelectedNode = nullptr;

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
	public:
		Vector3 mSelectedNodePosition = Vector3(0, 0, 0);
		Vector3 mSelectedNodeEulerAngles = Vector3(0, 0, 0);
		Vector3 mSelectedNodeScale = Vector3(1, 1, 1);

		std::vector<MaterialGui> mMaterialsGui = {};
	};
}
