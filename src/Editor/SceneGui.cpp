#include "tspch.h"
#include "SceneGui.h"
#include <Core/Factory.h>

namespace TS_ENGINE {

	static std::filesystem::path mAssetsPath = "Assets";

	SceneGui::SceneGui()
	{
		mTransformComboItems[0] = "Local";
		mTransformComboItems[1] = "World";

		mMeshEditorIcon = TS_ENGINE::Texture2D::Create("Resources\\Gui\\MeshEditor.png");
		//mMeshEditorIcon->SetVerticalFlip(false);
		mMaterialEditorIcon = TS_ENGINE::Texture2D::Create("Resources\\Gui\\LitMaterialIcon.png");
		//mMaterialEditorIcon->SetVerticalFlip(false);

		mContentBrowserDirectoryIcon = TS_ENGINE::Texture2D::Create("Resources\\Gui\\ContentBrowserDirectoryIcon.png");
		//mContentBrowserDirectoryIcon->SetVerticalFlip(false);
		mContentBrowserModelFileIcon = TS_ENGINE::Texture2D::Create("Resources\\Gui\\ContentBrowserModelFileIcon.png");
		//mContentBrowserModelFileIcon->SetVerticalFlip(false);
		mContentBrowserImageFileIcon = TS_ENGINE::Texture2D::Create("Resources\\Gui\\ContentBrowserImageFileIcon.png");
		//mContentBrowserImageFileIcon->SetVerticalFlip(false);
		mContentBrowserShaderFileIcon = TS_ENGINE::Texture2D::Create("Resources\\Gui\\ContentBrowserShaderFileIcon.png");
		//mContentBrowserShaderFileIcon->SetVerticalFlip(false);
		mContentBrowserMiscFileIcon = TS_ENGINE::Texture2D::Create("Resources\\Gui\\ContentBrowserMiscFileIcon.png");
		//mContentBrowserMiscFileIcon->SetVerticalFlip(false);

		//mUnlockedIcon = TS_ENGINE::Texture2D::Create("Assets\\Textures\\Gui\\Unlocked.png");
		//mUnlockedIcon->SetVerticalFlip(false);
		//mLockedIcon = TS_ENGINE::Texture2D::Create("Assets\\Textures\\Gui\\Locked.png");
		//mLockedIcon->SetVerticalFlip(false);	
		//mMeshFilterIcon = TS_ENGINE::Texture2D::Create("Assets\\Textures\\Gui\\MeshFilterIcon.png");
		//mMeshRendererIcon = TS_ENGINE::Texture2D::Create("Assets\\Textures\\Gui\\MeshRendererIcon.png");
		//mMaterialIcon = TS_ENGINE::Texture2D::Create("Assets\\Textures\\Gui\\MaterialIcon.png");
		//mLitMaterialIcon = TS_ENGINE::Texture2D::Create("Assets\\Textures\\Gui\\LitMaterialIcon.png");

		mCurrentDirectory = mAssetsPath;
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

	void SceneGui::ShowViewportWindow(Ref<TS_ENGINE::Camera> editorCamera, Ref<TS_ENGINE::Camera> currentSceneCamera)
	{
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
				DropItemInViewport();
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
		ImGui::Begin("Inspector");
		{
			if (mSelectedNode != NULL)
			{
				Ref<TS_ENGINE::Transform> transform = mSelectedNode->GetTransform();

				ImGui::Checkbox(" ", &GetSelectedNode()->m_Enabled);
				ImGui::SameLine();
				ImGui::Text(GetSelectedNode()->GetName().c_str());

#pragma region Transform Component
				ImGui::BeginChild("Transform", ImVec2(ImGui::GetWindowSize().x - 30.0f, 128.0f), true);

				float transformTextPosY = ImGui::GetCursorPosY();
				ImGui::Text("Transform");
				ImGui::SameLine();

				ImGui::SetCursorPosY(transformTextPosY - 2.5f);
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

				if (mSelectedNode != NULL && mSelectedNode->GetMeshes().size() > 0)
				{
					//switch (mSelectedNode->GetEntityType())
					{
						//case TS_ENGINE::EntityType::GAMEOBJECT:
						{
#pragma region Mesh Editor
							ImGui::BeginChild("Mesh Container", ImVec2(ImGui::GetWindowSize().x - 30.0f, 75.0f), true);

							float meshEditorIconPosY = ImGui::GetCursorPosY();
							ImGui::Image((void*)(intptr_t)mMeshEditorIcon->GetRendererID(), ImVec2(20, 20));
							ImGui::SameLine();

							ImGui::SameLine();
							ImGui::SetCursorPosY(meshEditorIconPosY + 3.5f);
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
							ImGui::Spacing();
							ImGui::SetNextItemWidth(100);
							float meshHeaderTextY = ImGui::GetCursorPosY();
							ImGui::Text("Mesh");
							ImGui::SameLine();

							ImGui::SetCursorPosY(meshHeaderTextY - 2.5f);
							if (ImGui::BeginCombo("##Mesh", mCurrentMeshItem))
							{
								for (int n = 0; n < IM_ARRAYSIZE(mMeshNameList); n++)
								{
									bool is_selected = (mCurrentMeshItem == mMeshNameList[n]); // You can store your selection however you want, outside or inside your objects

									if (ImGui::Selectable(mMeshNameList[n], is_selected))
									{
										mCurrentMeshItem = mMeshNameList[n];
										TS_CORE_INFO("Changing mesh to: {0}", mCurrentMeshItem);
										//TS_ENGINE::Factory::GetInstance()->ChangeMeshForNode(mSelectedNode, n);

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
							ImGui::BeginChild("Material Editor", ImVec2(ImGui::GetWindowSize().x - 30.0f, mSelectedNode->GetMeshes().size() * 420.0f), true);

							float materialIconPosY = ImGui::GetCursorPosY();
							ImGui::Image((void*)(intptr_t)mMaterialEditorIcon->GetRendererID(), ImVec2(20, 20));
							ImGui::SameLine();
							ImGui::SetCursorPosY(materialIconPosY + 3.5f);
							ImGui::Text("Material Editor");

							ShowAllMaterials();

							ImGui::EndChild();
#pragma endregion
						}
						//break;

						//case TS_ENGINE::EntityType::CAMERA:
						//{
							//TODO
						//}
						//break;

						//case TS_ENGINE::EntityType::LIGHT:
						//{
							//TODO
						//}
						//break;

						//case TS_ENGINE::EntityType::DEFAULT:
						//{

						//}
						//break;

					}
				}

			}
			ImGui::End();
		}
	}

	void SceneGui::ShowAllMaterials()
	{
		for (int meshIndex = 0; meshIndex < mSelectedNode->GetMeshes().size(); meshIndex++)
		{			
			Ref<Mesh> mesh = mSelectedNode->GetMeshes()[meshIndex];
			MaterialGui materialGui = mMaterialsGui[meshIndex];
			
			ImGui::Separator();

			//Material name
			//ImGui::Spacing();
			ImGui::Text(mesh->GetMaterial()->GetName().c_str());
			ImGui::Separator();
			//ImGui::Spacing();

			//Ambient 
			ImGui::Text("Ambient");
			float* ambientColor = mesh->GetMaterial()->GetAmbientColor().data;
			if (ImGui::ColorEdit4((std::string("##AmbientColor") + std::to_string(meshIndex)).c_str(), ambientColor))
			{
				materialGui.mAmbientColor = Vector4(ambientColor[0], ambientColor[1], ambientColor[2], ambientColor[3]);
				mesh->GetMaterial()->SetAmbientColor(materialGui.mAmbientColor);
			}

			ImGui::Spacing();

			// Diffuse 
			ImGui::Text("Diffuse");
			// Color
			float* diffuseColor = mesh->GetMaterial()->GetDiffuseColor().data;
			if (ImGui::ColorEdit4((std::string("##DiffuseColor") + std::to_string(meshIndex)).c_str(), diffuseColor))
			{
				materialGui.mDiffuseColor = Vector4(diffuseColor[0], diffuseColor[1], diffuseColor[2], diffuseColor[3]);
				mesh->GetMaterial()->SetDiffuseColor(materialGui.mDiffuseColor);
			}

			ImGui::Spacing();

			// Texture
			if (mesh->GetMaterial()->GetDiffuseMap())
			{				
				ImGui::ImageButton((void*)(intptr_t)mesh->GetMaterial()->GetDiffuseMap()->GetRendererID(), ImVec2(40, 40));
				DropContentBrowserTexture(TextureType::DIFFUSE, materialGui, meshIndex);
			}
			else
			{				
				ImGui::ImageButton((void*)(intptr_t)0, ImVec2(40, 40));
				DropContentBrowserTexture(TextureType::DIFFUSE, materialGui, meshIndex);
			}
			ImGui::SameLine();
			//Offset and tiling
			ImGui::SetNextItemWidth(100.0f);
			ImVec2 diffuseMapOffsetUiPos = ImGui::GetCursorPos();
			if (ImGui::DragFloat2((std::string("##DiffuseMapOffset") + std::to_string(meshIndex)).c_str(), materialGui.mDiffuseMapOffset))
			{
				mesh->GetMaterial()->SetDiffuseMapOffset(Vector2(materialGui.mDiffuseMapOffset[0], materialGui.mDiffuseMapOffset[1]));
			}
			ImGui::SameLine();
			ImGui::Text("Offset");
			ImGui::SetCursorPos(ImVec2(diffuseMapOffsetUiPos.x + 0.0f, diffuseMapOffsetUiPos.y + 26.0f));
			ImGui::SetNextItemWidth(100.0f);
			if (ImGui::DragFloat2((std::string("##DiffuseMapTiling") + std::to_string(meshIndex)).c_str(), materialGui.mDiffuseMapTiling))
			{
				mesh->GetMaterial()->SetDiffuseMapTiling(Vector2(materialGui.mDiffuseMapTiling[0], materialGui.mDiffuseMapTiling[1]));
			}
			ImGui::SameLine();
			ImGui::Text("Tiling");

			ImGui::Spacing();

			// Specular 
			ImGui::Text("Specular");
			// Color
			float* specularColor = mesh->GetMaterial()->GetSpecularColor().data;
			if (ImGui::ColorEdit4((std::string("##SpecularColor") + std::to_string(meshIndex)).c_str(), specularColor))
			{
				materialGui.mSpecularColor = Vector4(specularColor[0], specularColor[1], specularColor[2], specularColor[3]);
				mesh->GetMaterial()->SetSpecularColor(materialGui.mSpecularColor);
			}

			ImGui::Spacing();

			// Texture
			if (mesh->GetMaterial()->GetSpecularMap())
			{
				ImGui::ImageButton((void*)(intptr_t)mesh->GetMaterial()->GetSpecularMap()->GetRendererID(), ImVec2(40, 40));
				DropContentBrowserTexture(TextureType::SPECULAR, materialGui, meshIndex);
			}
			else
			{
				ImGui::ImageButton((void*)(intptr_t)0, ImVec2(40, 40));
				DropContentBrowserTexture(TextureType::SPECULAR, materialGui, meshIndex);
			}
			ImGui::SameLine();
			//Offset and tiling
			ImGui::SetNextItemWidth(100.0f);
			ImVec2 specularMapOffsetUiPos = ImGui::GetCursorPos();
			if (ImGui::DragFloat2((std::string("##SpecularMapOffset") + std::to_string(meshIndex)).c_str(), materialGui.mSpecularMapOffset))
			{
				mesh->GetMaterial()->SetSpecularMapOffset(Vector2(materialGui.mSpecularMapOffset[0], materialGui.mSpecularMapOffset[1]));
			}
			ImGui::SameLine();
			ImGui::Text("Offset");
			ImGui::SetCursorPos(specularMapOffsetUiPos + ImVec2(0.0f, 26.0f));
			ImGui::SetNextItemWidth(100.0f);
			if (ImGui::DragFloat2((std::string("##SpecularMapTiling") + std::to_string(meshIndex)).c_str(), materialGui.mSpecularMapTiling))
			{
				mesh->GetMaterial()->SetSpecularMapTiling(Vector2(materialGui.mSpecularMapTiling[0], materialGui.mSpecularMapTiling[1]));
			}
			ImGui::SameLine();
			ImGui::Text("Tiling");

			ImGui::Spacing();

			// Shininess Slider
			if (ImGui::SliderFloat((std::string("##Shininess") + std::to_string(meshIndex)).c_str(), &materialGui.mShininess, 0, 20.0f))
			{
				mesh->GetMaterial()->SetBumpValue(materialGui.mShininess);
			}

			ImGui::Spacing();

			// Normal
			ImGui::Text("Normal");
			//Texture						
			if (mesh->GetMaterial()->GetNormalMap())
			{
				ImGui::ImageButton((void*)(intptr_t)mesh->GetMaterial()->GetNormalMap()->GetRendererID(), ImVec2(40, 40));
				DropContentBrowserTexture(TextureType::NORMAL, materialGui, meshIndex);
			}
			else
			{
				ImGui::ImageButton((void*)(intptr_t)0, ImVec2(40, 40));
				DropContentBrowserTexture(TextureType::NORMAL, materialGui, meshIndex);
			}
			ImGui::SameLine();
			//Offset and tiling
			ImGui::SetNextItemWidth(100.0f);
			ImVec2 normalMapOffsetUiPos = ImGui::GetCursorPos();
			if (ImGui::DragFloat2((std::string("##NormalMapOffset") + std::to_string(meshIndex)).c_str(), materialGui.mNormalMapOffset))
			{
				mesh->GetMaterial()->SetNormalMapOffset(Vector2(materialGui.mNormalMapOffset[0], materialGui.mNormalMapOffset[1]));
			}
			ImGui::SameLine();
			ImGui::Text("Offset");
			ImGui::SetCursorPos(normalMapOffsetUiPos + ImVec2(0.0f, 26.0f));
			ImGui::SetNextItemWidth(100.0f);
			if (ImGui::DragFloat2((std::string("##NormalMapTiling") + std::to_string(meshIndex)).c_str(), materialGui.mNormalMapTiling))
			{
				mesh->GetMaterial()->SetNormalMapTiling(Vector2(materialGui.mNormalMapTiling[0], materialGui.mNormalMapTiling[1]));
			}
			ImGui::SameLine();
			ImGui::Text("Tiling");

			ImGui::Spacing();

			// Bump Slider
			if (ImGui::SliderFloat((std::string("##Bump") + std::to_string(meshIndex)).c_str(), &materialGui.mBumpValue, 0, 20.0f))
			{
				mesh->GetMaterial()->SetBumpValue(materialGui.mBumpValue);
			}

			//ImGui::EndChild();
		}
	}

	void SceneGui::ShowContentBrowser()
	{
		ImGui::Begin("ContentBrowser", 0, ImGuiWindowFlags_HorizontalScrollbar);
		{
			if (mCurrentDirectory != std::filesystem::path(mAssetsPath))
			{
				if (ImGui::Button("<-"))
				{
					mCurrentDirectory = mCurrentDirectory.parent_path();
				}
			}

			for (auto& directoryEntry : std::filesystem::directory_iterator(mCurrentDirectory))
			{
				const auto& path = directoryEntry.path();
				auto relativePath = std::filesystem::relative(path, mAssetsPath);
				std::string filenameStr = relativePath.filename().string();

				float buttonSize = 128.0f;
				float spacing = 50.0f;
				float iconSize = buttonSize * 0.75f;

				if (directoryEntry.is_directory())
				{
					ImVec2 cursorPos = ImGui::GetCursorScreenPos();

					if (ImGui::Button(("##" + filenameStr).c_str(), ImVec2(buttonSize, buttonSize)))
					{
						mCurrentDirectory /= path.filename();
					}

					ImGui::SameLine();
					ImVec2 imagePos = cursorPos + ImVec2((buttonSize - iconSize) * 0.5f, (buttonSize - iconSize) * 0.5f);
					ImGui::SetCursorScreenPos(imagePos);
					ImGui::Image((void*)(intptr_t)mContentBrowserDirectoryIcon->GetRendererID(), ImVec2(iconSize, iconSize), { 0, 1 }, { 1, 0 });

					ImGui::SameLine();

					if (std::strlen(filenameStr.c_str()) > 15)//Truncate string to 15 characters
					{
						filenameStr = Utility::GetTruncatedString(filenameStr, 15);
					}

					ImVec2 textPos = cursorPos + ImVec2((buttonSize - ImGui::CalcTextSize(filenameStr.c_str()).x) * 0.5f, buttonSize);
					ImGui::SetCursorScreenPos(textPos);
					ImGui::Text(filenameStr.c_str());

					ImGui::SameLine();
					ImGui::SetCursorScreenPos(cursorPos + ImVec2(buttonSize + spacing, 0));
				}
				else
				{
					std::string fileName;
					std::string fileExtension;
					Utility::GetFilenameAndExtension(filenameStr, fileName, fileExtension);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();

					if (ImGui::Button(("##" + filenameStr).c_str(), ImVec2(buttonSize, buttonSize)))
					{

					}

					ImGui::SameLine();
					ImGui::SetCursorScreenPos(cursorPos);

					ImVec2 imagePos = cursorPos + ImVec2((buttonSize - iconSize) * 0.5f, (buttonSize - iconSize) * 0.5f);
					ImGui::SetCursorScreenPos(imagePos);

					if (fileExtension == "png" || fileExtension == "jpg")
					{
						DragContentBrowserItem(path.string().c_str(), ItemType::TEXTURE);

						ImGui::Image((void*)(intptr_t)mContentBrowserImageFileIcon->GetRendererID(), ImVec2(iconSize, iconSize), { 0, 1 }, { 1, 0 });

						ImGui::SameLine();
						ImVec2 textPos = cursorPos + ImVec2((buttonSize - ImGui::CalcTextSize(fileExtension.c_str()).x) * 0.5f - 5.0f, buttonSize - ImGui::CalcTextSize(fileName.c_str()).y - 30.0f);
						ImGui::SetCursorScreenPos(textPos);
						ImGui::Text(("." + fileExtension).c_str());
					}
					else if (fileExtension == "vert" || fileExtension == "frag")
					{
						ImGui::Image((void*)(intptr_t)mContentBrowserShaderFileIcon->GetRendererID(), ImVec2(iconSize, iconSize), { 0, 1 }, { 1, 0 });
					}
					else if (fileExtension == "obj" || fileExtension == "stl" || fileExtension == "fbx" || fileExtension == "glb" || fileExtension == "gltf")
					{
						DragContentBrowserItem(path.string().c_str(), ItemType::MODEL);

						ImGui::Image((void*)(intptr_t)mContentBrowserModelFileIcon->GetRendererID(), ImVec2(iconSize, iconSize), { 0, 1 }, { 1, 0 });
					}
					else
					{
						ImGui::Image((void*)(intptr_t)mContentBrowserMiscFileIcon->GetRendererID(), ImVec2(iconSize, iconSize), { 0, 1 }, { 1, 0 });
					}

					ImGui::SameLine();

					if (std::strlen(fileName.c_str()) > 15)//Truncate string to 15 characters
					{
						fileName = Utility::GetTruncatedString(fileName, 15);
					}

					ImVec2 textPos = cursorPos + ImVec2((buttonSize - ImGui::CalcTextSize(fileName.c_str()).x) * 0.5f, buttonSize);
					ImGui::SetCursorScreenPos(textPos);
					ImGui::Text(fileName.c_str());

					ImGui::SameLine();
					ImGui::SetCursorScreenPos(cursorPos + ImVec2(buttonSize + spacing, 0));
				}
			}

		}
		ImGui::End();
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

	void SceneGui::ShowHierarchyWindow(Ref<TS_ENGINE::Scene> scene)
	{
		ImGui::Begin("Hierarchy");
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);

			ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
			int nodeTreeGuiIndex = 0;

			if (ImGui::TreeNodeEx((void*)(intptr_t)nodeTreeGuiIndex, base_flags | ImGuiTreeNodeFlags_OpenOnArrow, scene->GetSceneNode()->GetName().c_str()))
			{
				nodeTreeGuiIndex++;

				DragHierarchySceneNode(scene->GetSceneNode().get());
				DropHierarchySceneNode(scene->GetSceneNode().get());

				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
				{
					SetSelectedNode(scene->GetSceneNode().get());
				}

				CreateUIForAllNodes(nodeTreeGuiIndex, scene->GetSceneNode().get());
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}

	void SceneGui::CreateUIForAllNodes(int& nodeTreeGuiIndex, TS_ENGINE::Node* node)
	{
		ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

		for (int i = 0; i < node->GetChildCount(); i++)
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);

			TS_ENGINE::Node* nodeChild = node->GetChildAt(i);

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
				if (ImGui::TreeNodeEx((void*)(intptr_t)nodeTreeGuiIndex, node_flags, nodeChild->GetName().c_str()))
				{
					nodeTreeGuiIndex++;

					DragHierarchySceneNode(nodeChild);
					DropHierarchySceneNode(nodeChild);

					if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
					{
						SetSelectedNode(nodeChild);
					}

					CreateUIForAllNodes(nodeTreeGuiIndex, nodeChild);
					ImGui::TreePop();
				}
			}
			else//Tree Leaves
			{
				node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

				if (nodeChild->IsVisibleInEditor())
				{
					if (ImGui::TreeNodeEx((void*)(intptr_t)nodeTreeGuiIndex, node_flags, nodeChild->GetName().c_str()))
					{
						nodeTreeGuiIndex++;

						DragHierarchySceneNode(nodeChild);
						DropHierarchySceneNode(nodeChild);

						if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
						{
							SetSelectedNode(nodeChild);
						}
					}
				}
			}
		}
	}

	void SceneGui::DragHierarchySceneNode(Node* node)
	{
		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("_TREENODE", node, sizeof(Node));
			ImGui::Text("Dragging: %s", node->GetName().c_str());
			ImGui::EndDragDropSource();
		}
	}
	void SceneGui::DragContentBrowserItem(const char* filePath, ItemType itemType)
	{
		if (ImGui::BeginDragDropSource())
		{
			if (itemType == ItemType::TEXTURE)
				ImGui::SetDragDropPayload("_CONTENTBROWSER_TEXTURE", filePath, strlen(filePath) * sizeof(const char*));
			else if (itemType == ItemType::MODEL)
				ImGui::SetDragDropPayload("_CONTENTBROWSER_MODEL", filePath, strlen(filePath) * sizeof(const char*));

			ImGui::Text("Dragging: %s", filePath);
			ImGui::EndDragDropSource();
		}
	}

	void SceneGui::DropHierarchySceneNode(Node* targetParentNode)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TREENODE"))
			{
				Node* draggingNode = reinterpret_cast<Node*>(payload->Data);
				TS_CORE_INFO("Dropped {0} on {1}", draggingNode->GetName().c_str(), targetParentNode->GetName().c_str());

				{//This code snippet multiplies the inverse transform matrix of old parent to node's transform to negate/undo the multiplication done earlier to handle proper transforms
					Matrix4 oldParentTransformMatrix = targetParentNode->GetTransform()->m_TransformationMatrix;
					Matrix4 transformMatrix = draggingNode->GetTransform()->m_TransformationMatrix;
					Matrix4 newTransformMatrix = glm::inverse(oldParentTransformMatrix) * transformMatrix;
					auto dd = Utility::Decompose(newTransformMatrix);

					draggingNode->GetTransform()->m_Pos = dd->translation;
					draggingNode->GetTransform()->m_EulerAngles = dd->eulerAngles * Vector3(57.2958f);
					draggingNode->GetTransform()->m_Scale = dd->scale;
				}

				draggingNode->SetParent(targetParentNode);
			}
			else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_CONTENTBROWSER_MODEL"))
			{
				const char* draggedModelPath = reinterpret_cast<const char*>(payload->Data);				
				Factory::GetInstance()->InstantiateModel(draggedModelPath, SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode().get());
			}
			ImGui::EndDragDropTarget();
		}
	}
	void SceneGui::DropContentBrowserTexture(TextureType textureType, MaterialGui& materialGui, int meshIndex)
	{
		Ref<Material> material = mSelectedNode->GetMeshes()[meshIndex]->GetMaterial();

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_CONTENTBROWSER_TEXTURE"))
			{
				const char* draggedTexturePath = reinterpret_cast<const char*>(payload->Data);
				TS_CORE_INFO("Dropped {0} on {1}", draggedTexturePath, "DiffuseTextureDropZone");

				Ref<Texture2D> texture = Texture2D::Create(draggedTexturePath);

				if (textureType == TextureType::DIFFUSE)
				{
					materialGui.mDiffuseMap = texture;
					material->SetDiffuseMap(texture);
				}
				else if (textureType == TextureType::SPECULAR)
				{
					materialGui.mSpecularMap = texture;
					material->SetSpecularMap(texture);
				}
				else if (textureType == TextureType::NORMAL)
				{
					materialGui.mNormalMap = texture;
					material->SetNormalMap(texture);
				}
			}

			ImGui::EndDragDropTarget();
		}
	}
	void SceneGui::DropItemInViewport()
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_CONTENTBROWSER_MODEL"))
			{
				const char* draggedModelPath = reinterpret_cast<const char*>(payload->Data);
				Factory::GetInstance()->InstantiateModel(draggedModelPath, SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode().get());
			}

			ImGui::EndDragDropTarget();
		}
	}

	void SceneGui::SetSelectedNode(TS_ENGINE::Node* node)
	{
		if (node != mSelectedNode)
		{
			mSelectedNode = node;
			mJustSelected = true;

			//if (node && node->HasAttachedObject())
			//{
			//	if (Ref<Object> object = node->GetAttachedObject())
			//	{
			//		//Mesh container
			//		switch (object->GetPrimitiveType())
			//		{
			//		case PrimitiveType::QUAD:
			//			mCurrentMeshItem = "Quad";
			//			break;
			//		case PrimitiveType::CUBE:
			//			mCurrentMeshItem = "Cube";
			//			break;
			//		case PrimitiveType::SPHERE:
			//			mCurrentMeshItem = "Sphere";
			//			break;
			//		case PrimitiveType::CONE:
			//			mCurrentMeshItem = "Cone";
			//			break;
			//		case PrimitiveType::CYLINDER:
			//			mCurrentMeshItem = "Cylinder";
			//			break;
			//		/*case PrimitiveType::MODEL:
			//			mCurrentMeshItem = "Model";
			//			break;*/
			//		case PrimitiveType::EMPTY:
			//			mCurrentMeshItem = "Empty";
			//			break;
			//		}

			// Mesh Renderer
			if (mSelectedNode)
			{
				mMaterialsGui.clear();

				for (int i = 0; i < mSelectedNode->GetMeshes().size(); i++)
				{
					MaterialGui materialGui;

					materialGui.mAmbientColor = mSelectedNode->GetMeshes()[i]->GetMaterial()->GetAmbientColor();
					materialGui.mDiffuseColor = mSelectedNode->GetMeshes()[i]->GetMaterial()->GetDiffuseColor();
					materialGui.mDiffuseMap = mSelectedNode->GetMeshes()[i]->GetMaterial()->GetDiffuseMap();
					materialGui.mDiffuseMapOffset = new float[2] { mSelectedNode->GetMeshes()[i]->GetMaterial()->GetDiffuseMapOffset().x, mSelectedNode->GetMeshes()[i]->GetMaterial()->GetDiffuseMapOffset().y };
					materialGui.mDiffuseMapTiling = new float[2] { mSelectedNode->GetMeshes()[i]->GetMaterial()->GetDiffuseMapTiling().x, mSelectedNode->GetMeshes()[i]->GetMaterial()->GetDiffuseMapTiling().y };
					materialGui.mSpecularColor = mSelectedNode->GetMeshes()[i]->GetMaterial()->GetSpecularColor();
					materialGui.mSpecularMap = mSelectedNode->GetMeshes()[i]->GetMaterial()->GetSpecularMap();
					materialGui.mSpecularMapOffset = new float[2] { mSelectedNode->GetMeshes()[i]->GetMaterial()->GetSpecularMapOffset().x, mSelectedNode->GetMeshes()[i]->GetMaterial()->GetSpecularMapOffset().y };
					materialGui.mSpecularMapTiling = new float[2] { mSelectedNode->GetMeshes()[i]->GetMaterial()->GetSpecularMapTiling().x, mSelectedNode->GetMeshes()[i]->GetMaterial()->GetSpecularMapTiling().y };
					materialGui.mShininess = mSelectedNode->GetMeshes()[i]->GetMaterial()->GetShininess();
					materialGui.mNormalMap = mSelectedNode->GetMeshes()[i]->GetMaterial()->GetNormalMap();
					materialGui.mNormalMapOffset = new float[2] { mSelectedNode->GetMeshes()[i]->GetMaterial()->GetNormalMapOffset().x, mSelectedNode->GetMeshes()[i]->GetMaterial()->GetNormalMapOffset().y };
					materialGui.mNormalMapTiling = new float[2] { mSelectedNode->GetMeshes()[i]->GetMaterial()->GetNormalMapTiling().x, mSelectedNode->GetMeshes()[i]->GetMaterial()->GetNormalMapTiling().y };
					materialGui.mBumpValue = mSelectedNode->GetMeshes()[i]->GetMaterial()->GetBumpValue();

					mMaterialsGui.push_back(materialGui);
				}
			}
		}
	}
}
