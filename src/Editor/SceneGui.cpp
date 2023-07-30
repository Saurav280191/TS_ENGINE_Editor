#include "tspch.h"
#include "SceneGui.h"

//#define GLM_FORCE_RADIANS

namespace TS_ENGINE {

	SceneGui::SceneGui()
	{
		mTransformComboItems[0] = "Local";
		mTransformComboItems[1] = "World";
		mMeshRendererIcon = TS_ENGINE::Texture2D::Create("Assets\\Textures\\Gui\\MeshRendererIcon.png");
		mMeshRendererIcon->SetVerticalFlip(false);
	}

	void SceneGui::ShowTransformGizmos(const float* view, const float* projection)
	{
		if (mSelectedNode)
		{
			Matrix4 modelMatrix = mSelectedNode->GetTransform()->GetTransformationMatrix();
			ImGuizmo::Manipulate(view, projection, mTransformOperation, mTransformMode, glm::value_ptr(modelMatrix));

			if (ImGuizmo::IsUsing() || mJustSelected)
			{
				mSelectedNode->UpdateTransformationMatrices(modelMatrix);
				Vector3 eulerAngles;
				Utility::DecomposeTransform(modelMatrix, mSelectedNodePosition, eulerAngles, mSelectedNodeScale);
				mSelectedNodeEulerAngles = eulerAngles * Vector3(57.2958f);

				mJustSelected = false;
			}
		}
	}

	void SceneGui::ShowViewportWindow(ImVec2 viewportPanelPos, ImVec2 viewportPanelSize, 
		Ref<TS_ENGINE::Camera> mEditorCamera, Ref<TS_ENGINE::Camera> mSceneCamera)
	{
#pragma region Editor camera viewport

		ImGui::SetNextWindowPos(viewportPanelPos);
		ImGui::SetNextWindowSize(viewportPanelSize);

		ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoDecoration);
		{	
			ImVec2 cameraFramebufferWindowSize = viewportPanelSize - ImVec2(0, 15.0f);

			//Camera framebuffer output image
			{
				uint64_t mEditorCameraRenderTextureID = mEditorCamera->GetFramebuffer()->GetColorAttachmentRendererID();
				ImGui::Image(reinterpret_cast<void*>(mEditorCameraRenderTextureID), cameraFramebufferWindowSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
			}

			//ImGuizmo
			{
				//ImGuizmo set perspective and drawlist
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();

				ImVec2 guizmoViewportPos = viewportPanelPos + ImVec2(0, 10.0f);
				ImVec2 guizmoViewportSize = cameraFramebufferWindowSize;

				ImGuizmo::SetRect(guizmoViewportPos.x, guizmoViewportPos.y, guizmoViewportSize.x, guizmoViewportSize.y);

				const float* projection = nullptr;
				const float* view = nullptr;

				if (mEditorCamera)
				{
					projection = glm::value_ptr(mEditorCamera->GetProjectionMatrix());
					view = glm::value_ptr(mEditorCamera->GetViewMatrix());
				}

				float* identityMatrix = (float*)glm::value_ptr(Matrix4(1));

				//if (mShowGrid)
				//	ImGuizmo::DrawGrid(view, projection, identityMatrix, 50.0f);

				//if (mShowTranformGizmo)
				ShowTransformGizmos(view, projection);
			}

			//Scene camera frambuffer image
			{
				ImVec2 sceneCameraFrameBufferViewportWindowSize = ImVec2(200.0f * TS_ENGINE::Application::Get().GetWindow().GetAspectRatio(), 200.0f);
				ImVec2 sceneCameraFrameBufferViewportWindowPos = viewportPanelPos + viewportPanelSize - sceneCameraFrameBufferViewportWindowSize - ImVec2(10, 10.0f);
				
				ImGui::SetCursorPos(sceneCameraFrameBufferViewportWindowPos - ImVec2(0, 50.0f));

				uint64_t textureID = mSceneCamera->GetFramebuffer()->GetColorAttachmentRendererID();
				ImGui::Image(reinterpret_cast<void*>(textureID), sceneCameraFrameBufferViewportWindowSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

				// Calculate border coordinates
				ImVec2 borderMin = sceneCameraFrameBufferViewportWindowPos;
				ImVec2 borderMax = sceneCameraFrameBufferViewportWindowPos + sceneCameraFrameBufferViewportWindowSize;

				// Draw the border
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				drawList->AddRect(borderMin, borderMax, IM_COL32(255, 255, 255, 255), 0.0f, 0, 3.0f);
			}
		}
		ImGui::End();

		/*ImGui::SetNextWindowPos(ImVec2(0, mViewportPanelSize.y + 40));
		ImGui::SetNextWindowSize(ImVec2(mViewportPanelSize.x, 40));

		ImGui::Begin("PickedColorTestWindow", &opened, defaultWindowFlags | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
		ImGui::ColorButton("##ColorButton", mPickedColor, ImGuiColorEditFlags_NoAlpha);
		ImGui::End();*/

#pragma endregion
	}

	void SceneGui::ShowStatsWindow(ImVec2 statsPanelPos, ImVec2 statsPanelSize)
	{
		ImGui::Begin("Stats");
		{
			ImGui::SetWindowSize(statsPanelSize);
			ImGui::SetWindowPos(statsPanelPos);

			/*if (ImGui::Checkbox("Batching enabled", &mScene1->m_BatchingEnabled))
			{
				if (mScene1->m_BatchingEnabled)
				{
					mSelectedNode = nullptr;
					mCurrentShader = mBatchLitShader;
				}
				else
				{
					mSelectedNode = nullptr;
					mScene1->OnUnBatched();

					for (auto& node : mNodes)
					{
						mScene1->GetSceneNode()->AddChild(mScene1->GetSceneNode(), node);
						node->SetParentNode(mScene1->GetSceneNode());
					}

					mCurrentShader = mDefaultShader;
				}

				mScene1->m_BatchButton.Click(mCurrentShader, mNodes);

				if (mScene1->m_BatchingEnabled)
					mScene1->OnBatched();
			}*/

			ImGui::Text("FPS: %.1f, %.3f ms/frame", 1000.0f / TS_ENGINE::Application::Get().GetDeltaTime(), TS_ENGINE::Application::Get().GetDeltaTime());

			ImGui::Text("Draw Calls: %d", TS_ENGINE::Application::Get().GetDrawCalls());
			ImGui::Text("Vertices: %d", TS_ENGINE::Application::Get().GetTotalVertices());
			ImGui::Text("Indices: %d", TS_ENGINE::Application::Get().GetTotalIndices());
		}
		ImGui::End();
	}

	void SceneGui::ShowInspectorWindow(ImVec2 inspectorPanelPos, ImVec2 inspectorPanelSize)
	{
		ImGuiWindowFlags inspectorWindowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

		ImGui::SetNextWindowPos(inspectorPanelPos);
		ImGui::SetNextWindowSize(inspectorPanelSize);		
		bool open = true;
		ImGui::Begin("Inspector", &open, inspectorWindowFlags);
		{
			if (GetSelectedNode() != NULL)
			{
				Ref<TS_ENGINE::Transform> transform = mSelectedNode->GetTransform();

				ImGui::Checkbox(" ", &GetSelectedNode()->m_Enabled);
				ImGui::SameLine();
				ImGui::Text(GetSelectedNode()->GetName().c_str());


#pragma Transform Component
				ImGui::BeginChild("Transform", ImVec2(inspectorPanelSize.x - 18, 128.0f), true, 
					inspectorWindowFlags | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
				{
					ImGui::Text("Transform");
					ImGui::SameLine();

					if (ImGui::Combo("Options", &mTransformCurrentItem, mTransformComboItems, IM_ARRAYSIZE(mTransformComboItems)))
					{
						switch (mTransformCurrentItem)
						{
						case 0:
							mTransformMode = ImGuizmo::MODE::LOCAL;
							break;
						case 1:
							mTransformMode = ImGuizmo::MODE::WORLD;
							break;
						}
					}

					if (ImGui::RadioButton("Tr", mTranslateActive))
					{
						mTranslateActive = true;
						mRotateActive = false;
						mScaleActive = false;

						mTransformOperation = ImGuizmo::OPERATION::TRANSLATE;
					}
					ImGui::SameLine();
					if (ImGui::RadioButton("Rt", mRotateActive))
					{
						mRotateActive = true;
						mTranslateActive = false;
						mScaleActive = false;

						mTransformOperation = ImGuizmo::OPERATION::ROTATE;
					}
					ImGui::SameLine();
					if (ImGui::RadioButton("Sc", mScaleActive))
					{
						mScaleActive = true;
						mTranslateActive = false;
						mRotateActive = false;

						mTransformOperation = ImGuizmo::OPERATION::SCALE;
					}

					//mSelectedModelPosition = glm::value_ptr(mSelectedNode->GetTransform()->GetLocalPosition());
					//mSelectedModelEulerAngles = glm::value_ptr(mSelectedNode->GetTransform()->GetLocalEulerAngles());
					//mSelectedModelScale = glm::value_ptr(mSelectedNode->GetTransform()->GetLocalScale());

					float* pos = (float*)glm::value_ptr(mSelectedNodePosition);
					float* eulerAngles = (float*)glm::value_ptr(mSelectedNodeEulerAngles);
					float* scale = (float*)glm::value_ptr(mSelectedNodeScale);

					//Postion		
					if (ImGui::DragFloat3("Position", pos))
					{
						mSelectedNode->GetTransform()->m_EulerAngles = Vector3(eulerAngles[0], eulerAngles[1], eulerAngles[2]);
						mSelectedNode->GetTransform()->m_Scale = Vector3(scale[0], scale[1], scale[2]);
						mSelectedNode->SetPosition(pos);
					}
					//EulerAngles
					if (ImGui::DragFloat3("Rotation", eulerAngles))
					{
						mSelectedNode->GetTransform()->m_Pos = Vector3(pos[0], pos[1], pos[2]);
						mSelectedNode->GetTransform()->m_Scale = Vector3(scale[0], scale[1], scale[2]);
						mSelectedNode->SetEulerAngles(eulerAngles);
					}
					//Scale
					if (ImGui::DragFloat3("Scale", scale))
					{
						mSelectedNode->GetTransform()->m_Pos = Vector3(pos[0], pos[1], pos[2]);
						mSelectedNode->GetTransform()->m_EulerAngles = Vector3(eulerAngles[0], eulerAngles[1], eulerAngles[2]);
						mSelectedNode->SetScale(scale);
					}
				}
				ImGui::EndChild();
#pragma endregion 

#pragma region MeshRenderer Component
				if (mSelectedNode != NULL && mSelectedNode->HasAttachedObject())
				{
					if (mSelectedNode->GetAttachedObject()->GetEntityType() == TS_ENGINE::GAMEOBJECT)
					{
						ImGui::BeginChild("Mesh Renderer", ImVec2(inspectorPanelSize.x - 18, 0.09259f * (float)TS_ENGINE::Application::Get().GetWindow().GetWidth()), true, 
							inspectorWindowFlags | ImGuiWindowFlags_NoScrollbar);

						ImGui::Image((ImTextureID)mMeshRendererIcon->GetRendererID(), ImVec2(20, 20));
						ImGui::SameLine();

						//if (mSelectedNode->GetAttachedObject()->GetIsInitialized())
						//	ImGui::Checkbox(" ", &mSelectedNode->GetAttachedObject()->IsMeshRendererActive);

						ImGui::SameLine();
						ImGui::Text("Mesh Renderer");

						/*if (ImGui::ListBox("Material", &mCurrentShaderIndex, mShaderNameList, 2, 1))
						{
							switch (mCurrentShaderIndex)
							{
							case 0:
								Logger::Print("Changing shader to Default");
								mIsCurrentMaterialLit = false;
								break;
							case 1:
								Logger::Print("Changing shader to Lit");
								mIsCurrentMaterialLit = true;
								break;
							}
						}*/

						//static const char* current_MaterialItem = "Default";
						//ImGui::SetNextItemWidth(100);
						//ImGui::Text("Material");
						//ImGui::SameLine();
						//if (ImGui::BeginCombo("##Material", current_MaterialItem))
						//{
						//	for (int n = 0; n < IM_ARRAYSIZE(mShaderNameList); n++)
						//	{
						//		bool is_selected = (current_MaterialItem == mShaderNameList[n]); // You can store your selection however you want, outside or inside your objects

						//		if (ImGui::Selectable(mShaderNameList[n], is_selected))
						//		{
						//			current_MaterialItem = mShaderNameList[n];
						//			TS_CORE_INFO("Changing material to: ", current_MaterialItem);
						//		}

						//		if (is_selected)
						//			ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
						//	}
						//	ImGui::EndCombo();
						//}

						ImGui::EndChild();
					}
				}
#pragma endregion

#pragma region Material Component
				//		if (mSelectedNode != NULL && mSelectedNode->GetAttachedObject())
			//		{
			//			if (mSelectedNode->GetAttachedObject()->GetEntityType() == TS_ENGINE::GAMEOBJECT)
			//			{
			//				Ref<GameObject> selectedNodeGameObject = static_cast<Ref<GameObject>>(mSelectedNode->GetAttachedObject());
			//			}
			//
			//			ImGui::BeginChild("Material", ImVec2(inspectorPanelSize.x - 18, 0.09259f * app.GetWindow().GetHeight()), true, window_flags | ImGuiWindowFlags_NoScrollbar);
			//
			//			ImVec4 albedoColor = ImVec4(
			//				mSelectedNode->GetGameObject()->GetMeshRenderer()->GetMaterial()->GetMaterialProperties()->GetAmbientColor().x,
			//				mSelectedNode->GetGameObject()->GetMeshRenderer()->GetMaterial()->GetMaterialProperties()->GetAmbientColor().y,
			//				mSelectedNode->GetGameObject()->GetMeshRenderer()->GetMaterial()->GetMaterialProperties()->GetAmbientColor().z,
			//				255);
			//
			//			//Show UI for material color
			//			if (mIsCurrentMaterialLit)
			//				ImGui::Image((ImTextureID)mLitMaterialIcon->GetRendererID(), ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), albedoColor);
			//			else
			//				ImGui::Image((ImTextureID)mMaterialIcon->GetRendererID(), ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), albedoColor);
			//			ImGui::SameLine();
			//			ImGui::Text("Material");
			//
			//			if (mSelectedNode->GetGameObject()->GetIsInitialized())
			//			{
			//				//ImGui::Image(0, ImVec2(20, 20));
			//				//ImGui::SameLine();
			//				ImGui::Text("Albedo");
			//				ImGui::SameLine();
			//
			//				float* ambientColor = new float[3] {albedoColor.x, albedoColor.y, albedoColor.z};
			//				float shininess = mSelectedNode->GetGameObject()->GetMeshRenderer()->GetMaterial()->GetMaterialProperties()->GetShininess();
			//
			//				ImGui::ColorEdit3("AlbedoColor", ambientColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			//				//ImGui::ColorEdit3("DiffuseColor", mSelectedNode->GetGameObject()->GetMeshRenderer()->GetMaterial()->GetMaterialProperties()->diffuseColor);
			//				//ImGui::ColorEdit3("SpecularColor", mSelectedNode->GetGameObject()->GetMeshRenderer()->GetMaterial()->GetMaterialProperties()->specularColor);
			//				//ImGui::SliderFloat("Opacity", &mSelectedNode->GetGameObject()->GetMeshRenderer()->GetMaterial()->GetMaterialProperties()->opacity, 0, 1);
			//				ImGui::SliderFloat("Shininess", &shininess, 0, 16);
			//
			//				mSelectedNode->GetAttachedObject()->GetMeshRenderer()->GetMaterial()->GetMaterialProperties()->SetAmbientColor(ambientColor[0], ambientColor[1], ambientColor[2]);
			//				mSelectedNode->GetAttachedObject()->GetMeshRenderer()->GetMaterial()->GetMaterialProperties()->SetShininess(shininess);
			//			}
			//			ImGui::EndChild();
			//		}
#pragma endregion

			}
			ImGui::End();
		}
	}
	
	void SceneGui::ShowHierarchyWindow(Ref<TS_ENGINE::Scene> scene, ImVec2 hierarchyPanelPos, ImVec2 hierarchyPanelSize)
	{
		ImGui::SetNextWindowPos(hierarchyPanelPos);
		ImGui::SetNextWindowSize(hierarchyPanelSize);
		bool open = true;
		ImGui::Begin("Hierarchy", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

			if (ImGui::TreeNodeEx((void*)(intptr_t)0, base_flags, scene->GetSceneNode()->GetName().c_str()))
			{
				CreateUIForAllNodes(scene->GetSceneNode());
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}

	Ref<Node> SceneGui::GetSelectedNode()
	{
		return mSelectedNode;
	}

	void SceneGui::SwitchToTranslateMode()
	{
		mTranslateActive = true;
		mRotateActive = false;
		mScaleActive = false;
		mTransformOperation = ImGuizmo::OPERATION::TRANSLATE;
	}

	void SceneGui::SwitchToRotateMode()
	{
		mTranslateActive = false;
		mRotateActive = true;
		mScaleActive = false;
		mTransformOperation = ImGuizmo::OPERATION::ROTATE;
	}

	void SceneGui::SwitchToScaleMode()
	{
		mTranslateActive = false;
		mRotateActive = false;
		mScaleActive = true;
		mTransformOperation = ImGuizmo::OPERATION::SCALE;
	}

	void SceneGui::CreateUIForAllNodes(const Ref<TS_ENGINE::Node> node)
	{
		static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		static int selectedChildNodeIndex = -1;

		for (int i = 0; i < node->GetChildCount(); i++)
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);

			Ref<TS_ENGINE::Node> nodeChild = node->GetChildAt(i);

			ImGuiTreeNodeFlags node_flags = base_flags;

#pragma region allChildrenHiddenCondition
			static bool allChildrenNodesHidden = true;

			for (auto child : nodeChild->GetChildren())
			{
				if (child->IsVisibleInEditor())
				{
					allChildrenNodesHidden = false;
					break;
				}
			}

			if (allChildrenNodesHidden)
				node_flags |= ImGuiTreeNodeFlags_Leaf;
#pragma endregion

			if (nodeChild->GetChildCount() > 0)
			{
				bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)selectedChildNodeIndex, node_flags, nodeChild->GetName().c_str());

				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
				{
					selectedChildNodeIndex = i;

					SetSelectedNode(nodeChild);
				}

				HandleNodeDragDrop(mSelectedNode, nodeChild);

				if (node_open)
				{
					CreateUIForAllNodes(nodeChild);
					ImGui::TreePop();
				}
			}
			else//Tree Leaves
			{
				node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
#ifdef TS_ENGINE_EDITOR
				if (nodeChild->IsVisibleInEditor())
#endif
				{
					ImGui::TreeNodeEx((void*)(intptr_t)selectedChildNodeIndex, node_flags, nodeChild->GetName().c_str());

					if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
					{
						selectedChildNodeIndex = i;

						SetSelectedNode(nodeChild);
					}
				}

				HandleNodeDragDrop(mSelectedNode, nodeChild);
			}
		}

	}

	void SceneGui::HandleNodeDragDrop(Ref<TS_ENGINE::Node> _pickedNode, Ref<TS_ENGINE::Node> _targetParentNode)
	{
		static Ref<TS_ENGINE::Node> pickedNode;
		static TS_ENGINE::Node* lastParentNode;
		static Matrix4 pickedNodeLastModelMatrix;

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
			//lastParentNode = _pickedNode->GetParentNodePtr();//TODO
			pickedNodeLastModelMatrix = _pickedNode->GetTransform()->GetTransformationMatrix();

			if (!pickedNode)
			{
				pickedNode = _pickedNode;
				TS_CORE_INFO("Picked node: " + pickedNode->GetName());
			}

			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TREENODE"))
			{
				//lastParentNode->RemoveChild(pickedNode);//TODO
				_targetParentNode->AddChild(pickedNode);
				TS_CORE_INFO("Accepted payload!");
			}
			else
			{
				TS_CORE_INFO("Dropped: " + pickedNode->GetName());
				TS_CORE_INFO("Payload not accepted!");
			}

			ImGui::EndDragDropTarget();
		}
	}

	void SceneGui::SetSelectedNode(Ref<TS_ENGINE::Node> node)
	{
		if (node != mSelectedNode)
		{
			mSelectedNode = node;
			mJustSelected = true;
		}
	}
}
