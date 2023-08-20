#pragma once
#include "SceneManager/Node.h"
#include "SceneManager/Scene.h"
#include <Renderer/Texture.h>
#include <GameObject.h>
#include <Application.h>
#include <Utils/Utility.h>
#include <Utils/MyMath.h>
#include <Factory.h>
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

		Ref<Node> GetSelectedNode()
		{
			return mSelectedNode;
		}

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
		enum ItemType
		{
			TEXTURE,
			MODEL,
			MATERIAL
		};
		enum TextureType
		{
			DIFFUSE,
			SPECUALR,
			NORMAL
		};
		
		void CreateUIForAllNodes(int& nodeTreeGuiIndex, const Ref<TS_ENGINE::Node> node);
		
		void DragHierarchySceneNode(Node* node);
		void DragContentBrowserItem(const char* filePath, ItemType itemType);
		
		void DropHierarchySceneNode(Node* targetParentNode);
		void DropContentBrowserTexture(TextureType textureType);
		void DropItemInViewport();

		Ref<Texture2D> mMeshEditorIcon;
		Ref<Texture2D> mMaterialEditorIcon;
		Ref<Texture2D> mContentBrowserDirectoryIcon;
		Ref<Texture2D> mContentBrowserModelFileIcon;
		Ref<Texture2D> mContentBrowserImageFileIcon;
		Ref<Texture2D> mContentBrowserShaderFileIcon;
		Ref<Texture2D> mContentBrowserMiscFileIcon;

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
		std::filesystem::path mCurrentDirectory;
	public:
		Vector3 mSelectedNodePosition;
		Vector3 mSelectedNodeEulerAngles;
		Vector3 mSelectedNodeScale;
		
		//Materail properties
		Vector4 mAmbientColor;
		Vector4 mDiffuseColor;
		Ref<Texture2D> mDiffuseMap;
		float testFloat = 0.0f;
		float testFloat1 = 0.0f;
		float* mDiffuseMapOffset;
		float* mDiffuseMapTiling;
		Vector4 mSpecularColor;
		Ref<Texture2D> mSpecularMap;
		float* mSpecularMapOffset;
		float* mSpecularMapTiling;
		float mShininess;
		Ref<Texture2D> mNormalMap;
		float* mNormalMapOffset;
		float* mNormalMapTiling;
		float mBumpValue;
	};
}
