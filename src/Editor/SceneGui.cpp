#include "tspch.h"
#include "SceneGui.h"
#include <Core/Factory.h>

namespace TS_ENGINE {

	//static std::filesystem::path mAssetsPath = "Assets";
	//static std::filesystem::path mResourcesPath = "Resources";
	//static std::filesystem::path mSavedScenesPath = "Resources\\SavedSceneThumbnails";

	SceneGui::SceneGui()
	{
		mEditorCameraRenderTextureID = 0;

		mTransformComboItems[0] = "Local";
		mTransformComboItems[1] = "World";

		mTakeSnap = false;
		mSnapshotPath = "";

		mUnlockedIcon = TS_ENGINE::Texture2D::Create(Application::s_ResourcesDir.string() +  "\\Gui\\Unlocked.png");// This is a texture which is not being used in the editor but is important to fix the smart pointer deletion issue. Check the core reason behind it.
		mMeshEditorIcon = TS_ENGINE::Texture2D::Create(TS_ENGINE::Application::s_ResourcesDir.string() + "\\Gui\\MeshEditor.png");
		mMaterialEditorIcon = TS_ENGINE::Texture2D::Create(TS_ENGINE::Application::s_ResourcesDir.string() + "\\Gui\\LitMaterialIcon.png");
		mCameraIcon = TS_ENGINE::Texture2D::Create(TS_ENGINE::Application::s_ResourcesDir.string() + "\\Gui\\Camera.png");

		mContentBrowserDirectoryIcon = TS_ENGINE::Texture2D::Create(Application::s_ResourcesDir.string() + "\\Gui\\ContentBrowserDirectoryIcon.png");
		mContentBrowserModelFileIcon = TS_ENGINE::Texture2D::Create(Application::s_ResourcesDir.string() + "\\Gui\\ContentBrowserModelFileIcon.png");
		mContentBrowserImageFileIcon = TS_ENGINE::Texture2D::Create(Application::s_ResourcesDir.string() + "\\Gui\\ContentBrowserImageFileIcon.png");
		mContentBrowserShaderFileIcon = TS_ENGINE::Texture2D::Create(Application::s_ResourcesDir.string() + "\\Gui\\ContentBrowserShaderFileIcon.png");
		mContentBrowserMiscFileIcon = TS_ENGINE::Texture2D::Create(Application::s_ResourcesDir.string() + "\\Gui\\ContentBrowserMiscFileIcon.png");
		mSceneFileIcon = TS_ENGINE::Texture2D::Create(Application::s_ResourcesDir.string() + "\\Gui\\SceneIcon.png");

		//mLockedIcon = TS_ENGINE::Texture2D::Create(Application::s_ResourcesDir.string() + "\\Gui\\Locked.png");
		//mMeshFilterIcon = TS_ENGINE::Texture2D::Create(Application::s_ResourcesDir.string() +  "\\Gui\\MeshFilterIcon.png");
		//mMeshRendererIcon = TS_ENGINE::Texture2D::Create(Application::s_ResourcesDir.string() +  "\\Gui\\MeshRendererIcon.png");
		//mMaterialIcon = TS_ENGINE::Texture2D::Create(Application::s_ResourcesDir.string() +  "\\Gui\\MaterialIcon.png");
		//mLitMaterialIcon = TS_ENGINE::Texture2D::Create(Application::s_ResourcesDir.string() +  "\\Gui\\LitMaterialIcon.png");

		mCurrentDirectory = Application::s_AssetsDir;

		for (auto& directoryEntry : std::filesystem::directory_iterator(Application::s_ThumbnailsDir))
		{
			if (!directoryEntry.is_directory())
			{
				const auto& path = directoryEntry.path();
				auto relativePath = std::filesystem::relative(path, Application::s_ThumbnailsDir);
				std::string filenameStr = relativePath.filename().string();
				std::string fileName = "";
				std::string fileExtension = "";
				Utility::GetFilenameAndExtension(filenameStr, fileName, fileExtension);

				if (fileExtension == "png")
				{
					//This is a scene thumbnail
					Ref<TS_ENGINE::Texture2D> thumbnailTex = TS_ENGINE::Texture2D::Create(path.string());
					mSavedSceneThumbnails.insert(std::pair<std::string, Ref<Texture2D>>(fileName, thumbnailTex));
				}
			}
		}
	}

	SceneGui::~SceneGui()
	{
		TS_CORE_INFO("Destroying SceneGui");
	}

	void SceneGui::ShowTransformGizmos(const float* view, const float* projection)
	{
		if (mSelectedNode)
		{
			Matrix4 globalTransformationMatrix = mSelectedNode->GetTransform()->GetGlobalTransformationMatrix();
			ImGuizmo::Manipulate(view, projection, mTransformOperation, mTransformMode, glm::value_ptr(globalTransformationMatrix));

			if (ImGuizmo::IsUsing() || mJustSelected)
			{
				mSelectedNode->GetTransform()->SetGlobalTransformationMatrix(globalTransformationMatrix);
				Matrix4 localTransformationMatrix = Matrix4(1);

				if (mSelectedNode->GetParentNode())
					localTransformationMatrix = glm::inverse(mSelectedNode->GetParentNode()->GetTransform()->GetGlobalTransformationMatrix()) * globalTransformationMatrix;
				else
					localTransformationMatrix = globalTransformationMatrix;

				mSelectedNode->GetTransform()->SetLocalTransformationMatrix(localTransformationMatrix);

				// Set mSelectedNodePosition, mSelectedNodeEulerAngles and mSelectedNodeScale for DragFloat GUI
				{
					Vector3 eulerAngles;
					Utility::DecomposeTransform(localTransformationMatrix, mSelectedNodeLocalPosition, eulerAngles, mSelectedNodeLocalScale);
					mSelectedNodeLocalEulerAngles = eulerAngles * Vector3(57.2958f);//To Degree
				}

				mSelectedNode->GetTransform()->SetLocalTransform(mSelectedNodeLocalPosition, mSelectedNodeLocalEulerAngles, mSelectedNodeLocalScale);

				mSelectedNode->UpdateTransformationMatrices(Matrix4(1));

				mJustSelected = false;
			}
		}
	}

	void SceneGui::ShowViewportWindow()
	{
		ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoDecoration);
		{
			if (ImGui::IsWindowHovered())
			{
				IsViewportActiveWindow = true;
			}
			ImVec2 cameraFramebufferWindowSize = ImGui::GetWindowSize();

			mViewportPos = ImGui::GetWindowPos();
			mViewportSize = ImGui::GetWindowSize();

			if (TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene())
			{
				if (Ref<EditorCamera> editorCamera = TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetEditorCamera())
				{
					//Resize editor camera's frambuffer size
					if (TS_ENGINE::FramebufferSpecification spec = editorCamera->GetFramebuffer()->GetSpecification();
						cameraFramebufferWindowSize.x > 0.0f && cameraFramebufferWindowSize.y > 0.0f && // zero sized framebuffer is invalid
						(spec.Width != cameraFramebufferWindowSize.x || spec.Height != cameraFramebufferWindowSize.y))
					{
						editorCamera->GetFramebuffer()->Resize((uint32_t)cameraFramebufferWindowSize.x, (uint32_t)cameraFramebufferWindowSize.y);
						//TS_CORE_INFO("Editor camera framebuffer size {0}, {1} ", 
						//	editorCamera->GetFramebuffer()->GetSpecification().Width, editorCamera->GetFramebuffer()->GetSpecification().Height);						
						mEditorCameraRenderTextureID = editorCamera->GetFramebuffer()->GetColorAttachmentRendererID();
					}
				}
			}

			//Camera framebuffer output image
			{
				mViewportImageRect = CreateRef<Rect>(mViewportPos.x, mViewportPos.y, mViewportSize.x, mViewportSize.y - 35);
				ImGui::Image(reinterpret_cast<void*>(mEditorCameraRenderTextureID), ImVec2(mViewportImageRect->w, mViewportImageRect->h), ImVec2(0, 1), ImVec2(1, 0));
				ImVec2 imageMin = ImGui::GetItemRectMin();
				ImVec2 imageMax = ImGui::GetItemRectMax();
				ImVec2 size = imageMax - imageMin;

				if (mTakeSnap && mSnapshotPath != "")
				{
					Ref<Rect> snapRect = CreateRef<Rect>(imageMin.x, imageMin.y + 185, size.x - 10, size.y - 50);

					CaptureSnapshot(snapRect, mSnapshotPath);
					mTakeSnap = false;
					mSnapshotPath = "";
				}

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

				if (TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene())
				{
					if (Ref<EditorCamera> editorCamera = TS_ENGINE::SceneManager::GetInstance()->GetCurrentScene()->GetEditorCamera())
					{
						projection = glm::value_ptr(editorCamera->GetProjectionMatrix());
						view = glm::value_ptr(editorCamera->GetViewMatrix());
					}
				}

				float* identityMatrix = (float*)glm::value_ptr(Matrix4(1));

				//if (mShowGrid)
				//	ImGuizmo::DrawGrid(view, projection, identityMatrix, 50.0f);
				//if (mShowTranformGizmo)

				ShowTransformGizmos(view, projection);
			}

			//Scene camera frambuffer image
			if (SceneManager::GetInstance()->GetCurrentScene())
			{
				if (SceneManager::GetInstance()->GetCurrentScene()->GetNumSceneCameras() > 0)
				{
					Ref<SceneCamera> currentSceneCamera = SceneManager::GetInstance()->GetCurrentScene()->GetCurrentSceneCamera();
					
					ImVec2 sceneCameraFramebufferViewportWindowSize = ImVec2(200.0f * TS_ENGINE::Application::Get().GetWindow().GetAspectRatio(), 200.0f);
					ImVec2 sceneCameraFramebufferViewportWindowPos = ImGui::GetWindowSize() - sceneCameraFramebufferViewportWindowSize - ImVec2(10, 15);

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
		}
		ImGui::End();

	}

	void SceneGui::ShowNewSceneWindow()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

		ImGui::SetNextWindowPos(ImVec2(Application::Get().GetWindow().GetWidth() / 2 - 200.0f, Application::Get().GetWindow().GetHeight() / 2 - 150.0f));
		ImGui::SetNextWindowSize(ImVec2(200, 100));

		ImGui::Begin("New Scene", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);
		{
			ImGui::SetNextItemWidth(200.0f);
			ImGui::SetCursorPosY(35.0f);
			ImGui::InputText("##NewSceneName", mNewSceneText, IM_ARRAYSIZE(mNewSceneText));
			ImGui::Spacing();

			ImGui::SetCursorPos(ImVec2(50.0f, 70.0f));
			if (ImGui::Button("CANCEL"))
			{
				m_ShowNewSceneWindow = false;
			}

			ImGui::SameLine();

			if (ImGui::Button("  OK  "))
			{
				//Flush old scene data
				mSelectedNode = nullptr;
				SceneManager::GetInstance()->FlushCurrentScene();

				//Create New Scene
				SceneManager::GetInstance()->CreateNewScene(mNewSceneText);

				// Reset new scene pop up text
				malloc((sizeof(mNewSceneText) + 1) * sizeof(char)); 
				memset(mNewSceneText, 0, (sizeof(mNewSceneText) + 1) * sizeof(char));
				strcpy_s(mNewSceneText, 256, "New Scene");

				m_ShowNewSceneWindow = false;
			}

			ImGui::End();
		}

		style.Colors[ImGuiCol_Border] = ImVec4(0.430000007f, 0.430000007f, 0.5f, 0.5f);
		//style.WindowBorderSize = 1.0f;		
	}

	void SceneGui::TakeSnapshot(const std::string& snapshotPath)
	{
		mSnapshotPath = snapshotPath;
		mTakeSnap = true;
	}

	void SceneGui::CaptureSnapshot(Ref<Rect> rect, std::string path)
	{
		// Create a buffer to store the pixel data
		std::vector<GLubyte> pixels((const uint64_t)(4.0f * rect->w * rect->h)); // Assuming RGBA format

		glReadPixels((GLsizei)rect->x, (GLsizei)rect->y, 
			(GLsizei)rect->w, (GLsizei)rect->h, 
			GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

		Ref<Image> image = CreateRef<Image>();
		image->pixels = pixels;
		image->width = (int)rect->w;
		image->height = (int)rect->h;

		image->WritePixelsToFile(path);

		// Set current pixels to corresponding texture in mSavedSceneThumbnails map
		const char* sceneName = SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode()->GetEntity()->GetName().c_str();
		auto it = mSavedSceneThumbnails.find(sceneName);
		
		Ref<Texture2D> latestSceneSnap = Texture2D::Create((uint32_t)rect->w, (uint32_t)rect->h);
		latestSceneSnap->SetData(pixels.data(), (uint32_t)(4.0f * rect->w * rect->h));

		if (it != mSavedSceneThumbnails.end())
		{		
			mSavedSceneThumbnails[sceneName] = latestSceneSnap;
			//mSavedSceneThumbnails[sceneName]->SetData(pixels.data(), 4 * rect->w * rect->h);// Should not change the data of the existing texture. 
																							  // If the viewport size changes the width and height of the texture will change.
		}
		else// Save thumbnail for the first time
		{
			mSavedSceneThumbnails.insert(std::pair<std::string, Ref<Texture2D>>(sceneName, latestSceneSnap));
		}
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

				if(ImGui::InputText("##NodeNameText", mSelectedNodeNameBuffer, IM_ARRAYSIZE(mSelectedNodeNameBuffer)))
				{
					mSelectedNode->GetEntity()->SetName(mSelectedNodeNameBuffer);
				}
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

				//float* pos = (float*)glm::value_ptr(mSelectedNodeLocalPosition);
				//mSelectedNodeEulerAngles = TS_ENGINE::MyMath::ClampEulerAngles(mSelectedNodeEulerAngles);//Will clamp the euler angles between 0 - 360
				//float* eulerAngles = (float*)glm::value_ptr(mSelectedNodeLocalEulerAngles);
				//float* scale = (float*)glm::value_ptr(mSelectedNodeLocalScale);

				//Postion		
				if (ImGui::DragFloat3("Position", glm::value_ptr(mSelectedNodeLocalPosition), 0.1f))
				{
					//mSelectedNode->GetTransform()->m_EulerAngles = Vector3(eulerAngles[0], eulerAngles[1], eulerAngles[2]);
					//mSelectedNode->GetTransform()->m_Scale = Vector3(scale[0], scale[1], scale[2]);
					//mSelectedNode->SetPosition(pos);
					mSelectedNode->GetTransform()->SetLocalPosition(mSelectedNodeLocalPosition);
					mSelectedNode->InitializeTransformMatrices();
				}
				//EulerAngles
				if (ImGui::DragFloat3("Rotation", glm::value_ptr(mSelectedNodeLocalEulerAngles), 0.1f))
				{
					//mSelectedNode->GetTransform()->m_Pos = Vector3(pos[0], pos[1], pos[2]);
					//mSelectedNode->GetTransform()->m_Scale = Vector3(scale[0], scale[1], scale[2]);
					//mSelectedNode->SetEulerAngles(eulerAngles);
					mSelectedNode->GetTransform()->SetLocalEulerAngles(mSelectedNodeLocalEulerAngles);
					mSelectedNode->InitializeTransformMatrices();
				}
				//Scale
				if (ImGui::DragFloat3("Scale", glm::value_ptr(mSelectedNodeLocalScale), 0.1f))
				{
					//mSelectedNode->GetTransform()->m_Pos = Vector3(pos[0], pos[1], pos[2]);
					//mSelectedNode->GetTransform()->m_EulerAngles = Vector3(eulerAngles[0], eulerAngles[1], eulerAngles[2]);
					//mSelectedNode->SetScale(scale);
					mSelectedNode->GetTransform()->SetLocalScale(mSelectedNodeLocalScale);
					mSelectedNode->InitializeTransformMatrices();
				}

				ImGui::EndChild();
#pragma endregion 

				if (mSelectedNode != NULL)
				{
					if (mSelectedNode->GetEntity()->GetEntityType() == EntityType::CAMERA)
					{
#pragma region Camera Editor
						ImGui::BeginChild("Camera", ImVec2(ImGui::GetWindowSize().x - 30.0f, 180.0f), true);

						float meshEditorIconPosY = ImGui::GetCursorPosY();
						ImGui::Image((void*)(intptr_t)mCameraIcon->GetRendererID(), ImVec2(30, 30), ImVec2(0, 1), ImVec2(1, 0));
						ImGui::SameLine();

						ImGui::SameLine();
						ImGui::SetCursorPosY(meshEditorIconPosY + 7.0f);
						ImGui::Text("Camera");
						ImGui::Separator();

						ImGui::Spacing();
						ImGui::SetNextItemWidth(100);
						float headerTextY = ImGui::GetCursorPosY();
						ImGui::Text("Projection");
						ImGui::SameLine();

						ImGui::SetCursorPosY(headerTextY - 2.5f);
						// Projection selector GUI
						if (ImGui::BeginCombo("##Projection", mCurrentProjection))
						{
							for (int n = 0; n < IM_ARRAYSIZE(mProjectionList); n++)
							{
								bool is_selected = (mCurrentProjection == mProjectionList[n]);

								if (ImGui::Selectable(mProjectionList[n], is_selected))
								{
									mCurrentProjection = mProjectionList[n];
									TS_CORE_INFO("Changing camera type to: {0}", mCurrentProjection);
									mSelectedNode->GetSceneCamera()->SetProjectionType((Camera::ProjectionType)n);
									mSelectedNode->GetSceneCamera()->RefreshFrustrumGUI();
									if (is_selected)
										ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
								}
							}
							ImGui::EndCombo();
						}

						ImGui::Separator();
						ImGui::Spacing();

						// Perspective camera GUI
						if (mSelectedNode->GetSceneCamera()->GetProjectionType() == Camera::ProjectionType::PERSPECTIVE)
						{
							ImGui::Text("Field Of View");
							ImGui::SameLine();
							float fov = mSelectedNode->GetSceneCamera()->GetPerspective().fov;
							ImGui::SetCursorPosX(165);
							if (ImGui::SliderFloat("##Field Of View", &fov, 30.0f, 90.0f))
							{
								mSelectedNode->GetSceneCamera()->SetFieldOfView(fov);
								mSelectedNode->GetSceneCamera()->RefreshFrustrumGUI();
							}
							ImGui::Separator();
							ImGui::Spacing();

							ImGui::Text("Clipping Planes");
							ImGui::SameLine();
							ImGui::Spacing();
							ImGui::SameLine();
							float nearTextPosX = ImGui::GetCursorPosX();
							ImGui::Text("Near");
							ImGui::SameLine();
							float zNear = mSelectedNode->GetSceneCamera()->GetPerspective().zNear;
							if (ImGui::DragFloat("##Near", &zNear, 0.1f, 0.1f, mSelectedNode->GetSceneCamera()->GetPerspective().zFar - 1))// 0.1  to FarPlane - 1
							{
								mSelectedNode->GetSceneCamera()->SetNearPlane(zNear);
								mSelectedNode->GetSceneCamera()->RefreshFrustrumGUI();
							}

							ImGui::SetCursorPosX(nearTextPosX);
							ImGui::Text("Far ");
							ImGui::SameLine();
							float zFar = mSelectedNode->GetSceneCamera()->GetPerspective().zFar;
							if (ImGui::DragFloat("##Far", &zFar, 0.1f, 0.1f, 1000.0f))// 0.1  to 1000
							{
								mSelectedNode->GetSceneCamera()->SetFarPlane(zFar);
								mSelectedNode->GetSceneCamera()->RefreshFrustrumGUI();
							}

							ImGui::Separator();
							ImGui::Spacing();
						}
						// Orthographics camera GUI
						else if (mSelectedNode->GetSceneCamera()->GetProjectionType() == Camera::ProjectionType::ORTHOGRAPHIC)
						{
							ImGui::Text("Size");
							ImGui::SameLine();
							float size = mSelectedNode->GetSceneCamera()->GetOrthographic().top;
							ImGui::SetCursorPosX(165);
							if (ImGui::DragFloat("##Size", &size, 0.1f, 0.1f, 50.0f))
							{
								mSelectedNode->GetSceneCamera()->SetOrthographicSize(size);
								mSelectedNode->GetSceneCamera()->RefreshFrustrumGUI();
							}

							ImGui::Separator();
							ImGui::Spacing();

							ImGui::Text("Clipping Planes");
							ImGui::SameLine();
							ImGui::Spacing();
							ImGui::SameLine();
							float nearTextPosX = ImGui::GetCursorPosX();
							ImGui::Text("Near");
							ImGui::SameLine();
							float zNear = mSelectedNode->GetSceneCamera()->GetOrthographic().zNear;
							if (ImGui::DragFloat("##Near", &zNear, 0.1f, 0.1f, mSelectedNode->GetSceneCamera()->GetOrthographic().zFar - 1))// 0.1  to FarPlane - 1
							{
								mSelectedNode->GetSceneCamera()->SetNearPlane(zNear);
								mSelectedNode->GetSceneCamera()->RefreshFrustrumGUI();
							}

							ImGui::SetCursorPosX(nearTextPosX);
							ImGui::Text("Far ");
							ImGui::SameLine();
							float zFar = mSelectedNode->GetSceneCamera()->GetOrthographic().zFar;
							if (ImGui::DragFloat("##Far", &zFar, 0.1f, 0.1f, 1000.0f))// 0.1  to 1000
							{
								mSelectedNode->GetSceneCamera()->SetFarPlane(zFar);
								mSelectedNode->GetSceneCamera()->RefreshFrustrumGUI();
							}

							ImGui::Separator();
							ImGui::Spacing();
						}

						ImGui::EndChild();
#pragma endregion
					}
					else if (mSelectedNode->GetMeshes().size() > 0)
					{
#pragma region Mesh Editor
						ImGui::BeginChild("Mesh Container", ImVec2(ImGui::GetWindowSize().x - 30.0f, 75.0f), true);

						float meshEditorIconPosY = ImGui::GetCursorPosY();
						ImGui::Image((void*)(intptr_t)mMeshEditorIcon->GetRendererID(), ImVec2(20, 20));
						ImGui::SameLine();

						ImGui::SameLine();
						ImGui::SetCursorPosY(meshEditorIconPosY + 3.5f);
						ImGui::Text("Mesh Container");

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
									if (std::string(mMeshNameList[n]) == "Model")
									{
										TS_CORE_TRACE("Upcoming feature. An option to set model path and refresh will be added!");// TODO
									}
									else
									{
										if (mSelectedNode->GetMeshes()[0]->GetPrimitiveType() != PrimitiveType::MODEL)//Avoid changing model's mesh
										{
											mCurrentMeshItem = mMeshNameList[n];
											TS_CORE_INFO("Changing mesh to: {0}", mCurrentMeshItem);
											mSelectedNode->ChangeMesh((PrimitiveType)(n + 1));// n + 1 because we are skipping index 0 which is for Line
										}
									}

									if (is_selected)
										ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
								}
							}
							ImGui::EndCombo();
						}

						ImGui::EndChild();
#pragma endregion

#pragma region Material Editor
						ImGui::BeginChild("Material Editor", ImVec2(ImGui::GetWindowSize().x - 30.0f, 0), true);
						{
							float materialIconPosY = ImGui::GetCursorPosY();
							ImGui::Image((void*)(intptr_t)mMaterialEditorIcon->GetRendererID(), ImVec2(20, 20));
							ImGui::SameLine();
							ImGui::SetCursorPosY(materialIconPosY + 3.5f);

							ImGui::Text("Material Editor");
							ImGui::Separator();

							//Show all materials
							ImGui::BeginChild("##Material Editor");
							{
								for (int meshIndex = 0; meshIndex < mSelectedNode->GetMeshes().size(); meshIndex++)
								{
									//Material tree will be collapsed when there are more than 1 meshes in the node
									mSelectedNode->GetMeshes()[meshIndex]->GetMaterial()->ShowGUI(meshIndex, mSelectedNode->GetMeshes().size() < 2);
								}
								ImGui::EndChild();
							}

							ImGui::EndChild();
						}
#pragma endregion
					}
				}

			}
			ImGui::End();
		}
	}

	void SceneGui::ShowContentBrowser()
	{
		ImGui::Begin("ContentBrowser", 0);
		{
			if (mCurrentDirectory != std::filesystem::path(Application::s_AssetsDir))
			{
				if (ImGui::Button("<-"))				
				{
					mCurrentDirectory = mCurrentDirectory.parent_path();
				}
			}			
			
			ImGui::BeginChild("ContentBrowserItems", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
			{
				for (auto& directoryEntry : std::filesystem::directory_iterator(mCurrentDirectory))
			{
				const auto& path = directoryEntry.path();
				auto relativePath = std::filesystem::relative(path, Application::s_AssetsDir);
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

					if (fileExtension == "png" || fileExtension == "jpg" || fileExtension == "jpeg" || fileExtension == "tga" || fileExtension == "hdr")
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
					else if (fileExtension == "obj" || 
						fileExtension == "stl" || fileExtension == "fbx" ||
						fileExtension == "FBX" || fileExtension == "glb" ||
						fileExtension == "gltf" || fileExtension == "blend" || 
						fileExtension == "3ds")
					{
						DragContentBrowserItem(path.string().c_str(), ItemType::MODEL);
						ImGui::Image((void*)(intptr_t)mContentBrowserModelFileIcon->GetRendererID(), ImVec2(iconSize, iconSize), { 0, 1 }, { 1, 0 });
					}
					else if (fileExtension == "scene")
					{
						DragContentBrowserItem(path.string().c_str(), ItemType::SCENE);

						auto it = mSavedSceneThumbnails.find(fileName);

						if (it != mSavedSceneThumbnails.end())
						{
							float thumbnailSizeX = buttonSize;
							float thumbnailSizeY = buttonSize * 0.5625f;
							ImVec2 thumbnailPos = cursorPos + ImVec2((buttonSize - thumbnailSizeX) * 0.5f, (buttonSize - thumbnailSizeY) * 0.5f);
							ImGui::SetCursorScreenPos(thumbnailPos);
							ImGui::Image((void*)(intptr_t)it->second->GetRendererID(), ImVec2(thumbnailSizeX, thumbnailSizeY), { 0, 1 }, { 1, 0 });
						}
						else
						{
							ImGui::Image((void*)(intptr_t)mSceneFileIcon->GetRendererID(), ImVec2(iconSize, iconSize), { 0, 1 }, { 1, 0 });
						}
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
			ImGui::EndChild();
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

	void SceneGui::ShowHierarchyWindow()
	{
		ImGui::Begin("Hierarchy");
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);

			ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;

			if (SceneManager::GetInstance()->GetCurrentScene())
			{
				if (ImGui::TreeNodeEx((void*)(intptr_t)SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode()->GetEntity()->GetEntityID(), node_flags, SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode()->GetEntity()->GetName().c_str()))
				{
					DragHierarchySceneNode(SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode());// Drag
					DropHierarchySceneNode(SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode());// Drop

					CreateUIForAllNodes(SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode());
					ImGui::TreePop();
				}
			}
		}
		ImGui::End();
	}

	void SceneGui::CreateUIForAllNodes(Ref<Node> node)
	{
		//Node context menu
		{
			if (ImGui::BeginPopup("NodePopUp"))
			{
				mNodePopedUp = true;

				if (ImGui::Button("Duplicate"))
				{
					mSelectedNode = nullptr;

					if (mHoveringOnNode->GetEntity()->GetEntityType() == EntityType::CAMERA)
					{
						Factory::GetInstance()->InstantitateDuplicateSceneCamera(mHoveringOnNode->GetSceneCamera());
					}
					else// Other nodes
					{
						mHoveringOnNode->GetParentNode()->AddChild(mHoveringOnNode->Duplicate());
						//TS_CORE_TRACE("Duplicating {0}", mHoveringOnNode->GetName().c_str());
					}

					ImGui::CloseCurrentPopup();
				}

				if (ImGui::Button("Delete"))
				{
					//Switch to another scene camera if mSelectedNode is scene camera node before deleting
					if (mHoveringOnNode->GetEntity()->GetEntityType() == EntityType::CAMERA)
					{
						TS_CORE_INFO("Total scene cameras are: {0}", SceneManager::GetInstance()->GetCurrentScene()->GetNumSceneCameras());

						if (SceneManager::GetInstance()->GetCurrentScene()->GetNumSceneCameras() > 1)
						{
							SceneManager::GetInstance()->GetCurrentScene()->RemoveSceneCamera(mHoveringOnNode->GetSceneCamera());
							SceneManager::GetInstance()->GetCurrentScene()->SwitchToAnotherSceneCamera(mHoveringOnNode->GetSceneCamera());
							mSelectedNode = nullptr;
							mHoveringOnNode->Destroy();
						}
						else// Other nodes
						{
							TS_CORE_ERROR("Can't delete the last scene camera");
						}
					}
					else
					{
						mSelectedNode = nullptr;
						mHoveringOnNode->Destroy();
					}
					//TS_CORE_TRACE("Deleting {0}", mHoveringOnNode->GetName().c_str());
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			else
			{
				mNodePopedUp = false;
			}
		}

		for (int i = 0; i < node->GetChildCount(); i++)
		{
			//ImGui::SetNextItemOpen(true, ImGuiCond_Once);

			Ref<Node> nodeChild = node->GetChildAt(i);

			ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

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

			if (allChildrenNodesHidden)// If all children hidden then leaf
				node_flags |= ImGuiTreeNodeFlags_Leaf;
#pragma endregion

			if (nodeChild->GetChildCount() > 0)
			{
				if (ImGui::TreeNodeEx((void*)(intptr_t)nodeChild->GetEntity()->GetEntityID(), node_flags, nodeChild->GetEntity()->GetName().c_str()))
				{
					//Node context menu pop up
					{
						if (!mNodePopedUp && ImGui::IsItemHovered())
						{
							mHoveringOnNode = nodeChild;
						}

						ImGui::OpenPopupOnItemClick("NodePopUp", ImGuiPopupFlags_MouseButtonRight);
					}

					DragHierarchySceneNode(nodeChild);
					DropHierarchySceneNode(nodeChild);

					if (ImGui::IsItemClicked())
					{
						SetSelectedNode(nodeChild);
					}

					CreateUIForAllNodes(nodeChild);
					ImGui::TreePop();
				}

				if (ImGui::IsItemClicked())
				{
					SetSelectedNode(nodeChild);
				}
			}
			else//Tree Leaves
			{
				node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

				if (nodeChild->IsVisibleInEditor())
				{
					if (ImGui::TreeNodeEx((void*)(intptr_t)nodeChild->GetEntity()->GetEntityID(), node_flags, nodeChild->GetEntity()->GetName().c_str()))
					{
						//Node context menu pop up
						{
							if (!mNodePopedUp && ImGui::IsItemHovered())
							{
								mHoveringOnNode = nodeChild;
							}
							ImGui::OpenPopupOnItemClick("NodePopUp", ImGuiPopupFlags_MouseButtonRight);
						}

						DragHierarchySceneNode(nodeChild);
						DropHierarchySceneNode(nodeChild);

						if (ImGui::IsItemClicked())
						{
							SetSelectedNode(nodeChild);
						}
					}
				}
			}

		}
	}

	void SceneGui::DragHierarchySceneNode(Ref<Node> node)
	{
		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("_TREENODE", node.get(), sizeof(Node));
			ImGui::Text("Dragging: %s", node->GetEntity()->GetName().c_str());
			ImGui::EndDragDropSource();
		}
	}

	void SceneGui::DragContentBrowserItem(const char* filePath, ItemType itemType)
	{
		if (ImGui::BeginDragDropSource())
		{
			if (itemType == ItemType::TEXTURE)
				ImGui::SetDragDropPayload("_CONTENTBROWSER_TEXTURE", filePath, (strlen(filePath) + 1) * (sizeof(const char*)));
			else if (itemType == ItemType::MODEL)
				ImGui::SetDragDropPayload("_CONTENTBROWSER_MODEL", filePath, (strlen(filePath) + 1) * (sizeof(const char*)));
			else if (itemType == ItemType::SCENE)
				ImGui::SetDragDropPayload("_CONTENTBROWSER_SCENE", filePath, (strlen(filePath) + 1) * (sizeof(const char*)));

			ImGui::Text("Dragging: %s", filePath);
			ImGui::EndDragDropSource();
		}
	}

	void SceneGui::DropHierarchySceneNode(Ref<Node> targetParentNode)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TREENODE"))
			{
				Node* draggingNode = reinterpret_cast<Node*>(payload->Data);
				TS_CORE_INFO("Dropped {0} on {1}", draggingNode->GetEntity()->GetName().c_str(), targetParentNode->GetEntity()->GetName().c_str());

				{
					Matrix4 transformMatrix = draggingNode->GetTransform()->GetGlobalTransformationMatrix();
					Matrix4 newTransformMatrix = glm::inverse(targetParentNode->GetTransform()->GetGlobalTransformationMatrix()) * transformMatrix;
					//Matrix4 newTransformMatrix = glm::inverse(draggingNode->GetTransform()->m_TransformationMatrix) * targetParentNode->GetTransform()->m_TransformationMatrix;// Ideal way

					auto dd = Utility::Decompose(newTransformMatrix);

					draggingNode->GetTransform()->m_Pos = dd->translation;
					draggingNode->GetTransform()->m_EulerAngles = dd->eulerAngles;
					draggingNode->GetTransform()->m_Scale = dd->scale;

					Matrix4 modelMatrix = draggingNode->GetTransform()->GetGlobalTransformationMatrix();
					draggingNode->UpdateTransformationMatrices(modelMatrix);
				}

				draggingNode->ChangeParent(targetParentNode);
			}
			else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_CONTENTBROWSER_MODEL"))
			{
				const char* draggedModelPath = reinterpret_cast<const char*>(payload->Data);
				Factory::GetInstance()->InstantiateModel(draggedModelPath, SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode());
			}
			ImGui::EndDragDropTarget();
		}
	}

	void SceneGui::DropItemInViewport()
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (SceneManager::GetInstance()->GetCurrentScene())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_CONTENTBROWSER_MODEL"))
				{
					const char* draggedModelPath = reinterpret_cast<const char*>(payload->Data);
					Factory::GetInstance()->InstantiateModel(draggedModelPath, SceneManager::GetInstance()->GetCurrentScene()->GetSceneNode());
				}
			}

			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_CONTENTBROWSER_SCENE"))
			{
				const char* draggedModelPath = reinterpret_cast<const char*>(payload->Data);
				
				mSelectedNode = nullptr;

				// Flush current scene
				SceneManager::GetInstance()->FlushCurrentScene();

				// Load dragged scene
				SceneManager::GetInstance()->LoadScene(draggedModelPath);
			}

			ImGui::EndDragDropTarget();
		}
	}

	void SceneGui::SetSelectedNode(Ref<Node> node)
	{
		if (node != mSelectedNode)
		{
			mSelectedNode = node;

			if (mSelectedNode != nullptr)
			{
				// Set mSelectedNodeLocalPosition, mSelectedNodeLocalEulerAngles and mSelectedNodeLocalScale after selection
				{
					if (mSelectedNode)
					{
						Quaternion rot;
						Utility::DecomposeMtx(mSelectedNode->GetTransform()->GetLocalTransformationMatrix(), mSelectedNodeLocalPosition, rot, mSelectedNodeLocalScale);
						mSelectedNodeLocalEulerAngles = glm::degrees(glm::eulerAngles(rot));
					}

					if (mSelectedNode)
					{
						// Copy the constant string into textBuffer
						strncpy(mSelectedNodeNameBuffer, mSelectedNode->GetEntity()->GetName().c_str(), sizeof(mSelectedNodeNameBuffer));

						// Ensure null-termination (if the string is shorter than the buffer)
						mSelectedNodeNameBuffer[sizeof(mSelectedNodeNameBuffer) - 1] = '\0';
					}

					mJustSelected = true;
				}

				// Set current scene's current camera. Update Camera properties in GUI
				{
					if (mSelectedNode->GetEntity()->GetEntityType() == EntityType::CAMERA)
					{
						SceneManager::GetInstance()->GetCurrentScene()->SetCurrentSceneCamera(mSelectedNode->GetSceneCamera());

						if (mSelectedNode->GetSceneCamera()->GetProjectionType() == Camera::ProjectionType::PERSPECTIVE)
						{
							mCurrentProjection = "Perspective";
						}
						else if (mSelectedNode->GetSceneCamera()->GetProjectionType() == Camera::ProjectionType::ORTHOGRAPHIC)
						{
							mCurrentProjection = "Orthographic";
						}
						else
						{
							mCurrentProjection = "Default";
						}
					}
				}

				// Update Mesh container and Mesh renderer after selection
				{
					if (mSelectedNode && mSelectedNode->HasMeshes())
					{
						//Mesh container
						{
							switch (mSelectedNode->GetMeshes()[0]->GetPrimitiveType())
							{
							case PrimitiveType::LINE:
								mCurrentMeshItem = "Line";
								break;
							case PrimitiveType::QUAD:
								mCurrentMeshItem = "Quad";
								break;
							case PrimitiveType::CUBE:
								mCurrentMeshItem = "Cube";
								break;
							case PrimitiveType::SPHERE:
								mCurrentMeshItem = "Sphere";
								break;
							case PrimitiveType::CYLINDER:
								mCurrentMeshItem = "Cylinder";
								break;
							case PrimitiveType::CONE:
								mCurrentMeshItem = "Cone";
								break;
							case PrimitiveType::MODEL:
								mCurrentMeshItem = "Model";
								break;
							}
						}

						// Mesh Renderer
						{
							for (int i = 0; i < mSelectedNode->GetMeshes().size(); i++)
							{
								Material::MaterialGui materialGui;

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

								mSelectedNode->GetMeshes()[i]->GetMaterial()->SetMaterialGui(materialGui);
							}
						}
					}
				}
			}
		}
	}

	void SceneGui::DeleteSelectedNode()
	{
		if (mSelectedNode)
		{
			// Switch to another scene camera if mSelectedNode is scene camera node
			if (mSelectedNode->GetEntity()->GetEntityType() == EntityType::CAMERA)
			{
				TS_CORE_INFO("Total scene cameras are: {0}", SceneManager::GetInstance()->GetCurrentScene()->GetNumSceneCameras());

				if (SceneManager::GetInstance()->GetCurrentScene()->GetNumSceneCameras() > 1)
				{
					SceneManager::GetInstance()->GetCurrentScene()->RemoveSceneCamera(mSelectedNode->GetSceneCamera());
					SceneManager::GetInstance()->GetCurrentScene()->SwitchToAnotherSceneCamera(mSelectedNode->GetSceneCamera());
					mSelectedNode->Destroy();
					mSelectedNode = nullptr;
				}
				else
				{
					TS_CORE_ERROR("Can't delete the last scene camera");
				}
			}
			else// Other nodes
			{
				mSelectedNode->Destroy();
				mSelectedNode = nullptr;
			}
		}
	}

	void SceneGui::DuplicatedSelectedNode()
	{
		if (mSelectedNode)
		{
			if (mSelectedNode->GetEntity()->GetEntityType() == EntityType::CAMERA)
			{
				Factory::GetInstance()->InstantitateDuplicateSceneCamera(mSelectedNode->GetSceneCamera());
			}
			else// Other nodes
			{
				mSelectedNode->GetParentNode()->AddChild(mSelectedNode->Duplicate());
			}
		}
	}
}
