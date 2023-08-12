#include "tspch.h"
#include "SceneGui.h"

//#define GLM_FORCE_RADIANS

namespace TS_ENGINE {

	SceneGui::SceneGui()
	{
		mTransformComboItems[0] = "Local";
		mTransformComboItems[1] = "World";
		mMeshEditorIcon = TS_ENGINE::Texture2D::Create("Assets\\Textures\\Gui\\MeshEditor.png");
		mMeshEditorIcon->SetVerticalFlip(false);
		mMaterialEditorIcon = TS_ENGINE::Texture2D::Create("Assets\\Textures\\Gui\\LitMaterialIcon.png");
		mMaterialEditorIcon->SetVerticalFlip(false);
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
				mSelectedNodeEulerAngles = eulerAngles * Vector3(57.2958f);//To Degree

				mJustSelected = false;
			}
		}
	}

	//Position of Viewport is: (0, 52)
	//Size of Viewport is: (WindowWidth - 250, WindowHeight - 52)
	void SceneGui::ShowViewportWindow(Ref<TS_ENGINE::Camera> editorCamera, Ref<TS_ENGINE::Camera> currentSceneCamera)
	{
#pragma region Editor camera viewport

		//ImGui::SetNextWindowPos(viewportPanelPos);
		//ImGui::SetNextWindowSize(viewportPanelSize);
		//ImGui::SetNextWindowPos(ImVec2(0, 18));
		ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoDecoration);// | ImGuiWindowFlags_NoDocking);
		{
			ImVec2 cameraFramebufferWindowSize = ImGui::GetWindowSize();

			mViewportPos = ImGui::GetWindowPos();
			mViewportSize = ImGui::GetWindowSize();
			mViewportBounds[0] = { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y };
			mViewportBounds[1] = { ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y };

			//Resize editor camera's frambuffer size
			if (TS_ENGINE::FramebufferSpecification spec = editorCamera->GetFramebuffer()->GetSpecification();
				cameraFramebufferWindowSize.x > 0.0f && cameraFramebufferWindowSize.y > 0.0f && // zero sized framebuffer is invalid
				(spec.Width != cameraFramebufferWindowSize.x || spec.Height != cameraFramebufferWindowSize.y))
			{
				editorCamera->GetFramebuffer()->Resize((uint32_t)cameraFramebufferWindowSize.x, (uint32_t)cameraFramebufferWindowSize.y);
			}

			//Camera framebuffer output image
			{
				uint64_t editorCameraRenderTextureID = editorCamera->GetFramebuffer()->GetColorAttachmentRendererID();
				//TS_CORE_INFO("Editor camera framebuffer size {0}, {1} ", 
				//	editorCamera->GetFramebuffer()->GetSpecification().Width, editorCamera->GetFramebuffer()->GetSpecification().Height);

				ImGui::Image(reinterpret_cast<void*>(editorCameraRenderTextureID), cameraFramebufferWindowSize, ImVec2(0, 1), ImVec2(1, 0));
			}

			//ImGuizmo
			{
				//ImGuizmo set perspective and drawlist
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();

				ImVec2 guizmoViewportPos = ImGui::GetWindowPos() + ImVec2(8.0f, 27.0f);//TODO: Found value (8.0f, 27.0f) to fix offset issue. Need to find the root cause of the issue.
				ImVec2 guizmoViewportSize = cameraFramebufferWindowSize;

				ImGuizmo::SetRect(guizmoViewportPos.x, guizmoViewportPos.y, guizmoViewportSize.x, guizmoViewportSize.y);

				const float* projection = nullptr;
				const float* view = nullptr;

				if (editorCamera)
				{
					projection = glm::value_ptr(editorCamera->GetProjectionMatrix());
					view = glm::value_ptr(editorCamera->GetViewMatrix());
				}

				float* identityMatrix = (float*)glm::value_ptr(Matrix4(1));

				//if (mShowGrid)
				//	ImGuizmo::DrawGrid(view, projection, identityMatrix, 50.0f);
				//if (mShowTranformGizmo)

				ShowTransformGizmos(view, projection);
			}

			//Scene camera frambuffer image
			if (currentSceneCamera)
			{
				ImVec2 sceneCameraFramebufferViewportWindowSize = ImVec2(200.0f * TS_ENGINE::Application::Get().GetWindow().GetAspectRatio(), 200.0f);
				ImVec2 sceneCameraFramebufferViewportWindowPos = ImGui::GetWindowSize() - sceneCameraFramebufferViewportWindowSize - ImVec2(10, 10);

				ImGui::SetCursorPos(sceneCameraFramebufferViewportWindowPos);

				uint64_t textureID = currentSceneCamera->GetFramebuffer()->GetColorAttachmentRendererID();
				ImGui::Image(reinterpret_cast<void*>(textureID), sceneCameraFramebufferViewportWindowSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

				// Calculate border coordinates
				ImVec2 borderMin = sceneCameraFramebufferViewportWindowPos + ImVec2(0, 20);
				ImVec2 borderMax = sceneCameraFramebufferViewportWindowPos + sceneCameraFramebufferViewportWindowSize + ImVec2(0, 20);

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

	void SceneGui::ShowInspectorWindow()
	{
		//ImGuiWindowFlags inspectorWindowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
		//ImGui::SetNextWindowPos(inspectorPanelPos);
		//ImGui::SetNextWindowSize(inspectorPanelSize);
		//bool open = true;
		//mSelectedNode = SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode()->GetChildAt(1);//Temporarily added for test

		ImGui::Begin("Inspector");
		{
			if (mSelectedNode != NULL)
			{
				Ref<TS_ENGINE::Transform> transform = mSelectedNode->GetTransform();

				ImGui::Checkbox(" ", &GetSelectedNode()->m_Enabled);
				ImGui::SameLine();
				ImGui::Text(GetSelectedNode()->GetName().c_str());

#pragma region Transform Component

				ImGui::BeginChild("Transform", ImVec2(ImGui::GetWindowSize().x, 128.0f), true);

				ImGui::Text("Transform");
				ImGui::SameLine();

				if (ImGui::Combo("##TransformMode", &mTransformCurrentItem, mTransformComboItems, IM_ARRAYSIZE(mTransformComboItems)))
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

				float* pos = (float*)glm::value_ptr(mSelectedNodePosition);
				//mSelectedNodeEulerAngles = TS_ENGINE::MyMath::ClampEulerAngles(mSelectedNodeEulerAngles);//Will clamp the euler angles between 0 - 360
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

				ImGui::EndChild();
#pragma endregion 

				if (mSelectedNode != NULL && mSelectedNode->HasAttachedObject())
				{
					switch (mSelectedNode->GetAttachedObject()->GetEntityType())
					{
					case TS_ENGINE::EntityType::GAMEOBJECT:
					{
#pragma region Mesh Editor
						ImGui::BeginChild("Mesh Container", ImVec2(ImGui::GetWindowSize().x, 128.0f), true);

						ImGui::Image((ImTextureID)mMeshEditorIcon->GetRendererID(), ImVec2(20, 20));
						ImGui::SameLine();

						ImGui::SameLine();
						ImGui::Text("Mesh Container");

						/*if (ImGui::ListBox("Mesh", &mCurrentMeshIndex, mMeshNameList, 2, 1))
						{
							switch (mCurrentMeshIndex)
							{
							case 0:

								break;
							case 1:

								break;
							}
						}*/

						ImGui::SetNextItemWidth(100);
						ImGui::Text("Mesh");
						ImGui::SameLine();
						if (ImGui::BeginCombo("##Mesh", mCurrentMeshItem))
						{
							for (int n = 0; n < IM_ARRAYSIZE(mMeshNameList); n++)
							{
								bool is_selected = (mCurrentMeshItem == mMeshNameList[n]); // You can store your selection however you want, outside or inside your objects

								if (ImGui::Selectable(mMeshNameList[n], is_selected))
								{
									mCurrentMeshItem = mMeshNameList[n];
									TS_CORE_INFO("Changing mesh to: {0}", mCurrentMeshItem);
									TS_ENGINE::Factory::GetInstance()->ChangeMeshForNode(mSelectedNode, n);

									//"Quad",
									//"Cube",
									//"Sphere",
									//"Cone",
									//"Cylinder",
									//"Model",
									//"Empty"
								}

								if (is_selected)
									ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
							}
							ImGui::EndCombo();
						}

						ImGui::EndChild();
#pragma endregion

#pragma region Material Editor
						ImGui::BeginChild("Material Editor", ImVec2(ImGui::GetWindowSize().x, 128.0f), true);

						ImGui::Image((ImTextureID)mMaterialEditorIcon->GetRendererID(), ImVec2(20, 20));
						ImGui::SameLine();
						ImGui::Text("Material Editor");

						ImGui::EndChild();
#pragma endregion
					}
					break;

					case TS_ENGINE::EntityType::CAMERA:
					{
						//TODO
					}
					break;

					case TS_ENGINE::EntityType::LIGHT:
					{
						//TODO
					}
					break;

					case TS_ENGINE::EntityType::DEFAULT:
					{

					}
					break;

					}
				}

			}
			ImGui::End();
		}
	}

	void SceneGui::ShowHierarchyWindow(Ref<TS_ENGINE::Scene> scene)
	{
		//ImGui::SetNextWindowPos(hierarchyPanelPos);
		//ImGui::SetNextWindowSize(hierarchyPanelSize);
		//bool open = true;
		ImGui::Begin("Hierarchy");// , & open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
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

	void SceneGui::ShowContentBrowser()
	{
		ImGui::Begin("ContentBrowser");
		{

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

			if (node && node->HasAttachedObject())
			{
				if (Ref<Object> object = node->GetAttachedObject())
				{
					switch (object->GetPrimitiveType())
					{
					case PrimitiveType::QUAD:
						mCurrentMeshItem = "Quad";
						break;
					case PrimitiveType::CUBE:
						mCurrentMeshItem = "Cube";
						break;
					case PrimitiveType::SPHERE:
						mCurrentMeshItem = "Sphere";
						break;
					case PrimitiveType::CONE:
						mCurrentMeshItem = "Cone";
						break;
					case PrimitiveType::CYLINDER:
						mCurrentMeshItem = "Cylinder";
						break;
					case PrimitiveType::MODEL:
						mCurrentMeshItem = "Model";
						break;
					case PrimitiveType::EMPTY:
						mCurrentMeshItem = "Empty";
						break;
					}
				}
			}
		}
	}
}
