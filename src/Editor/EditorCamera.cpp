#include <tspch.h>
#include "EditorCamera.h"
#include <Core/Input.h>
#include <GLM/gtx/quaternion.hpp>
#include <Primitive/Sphere.h>
#include <SceneManager/Node.h>

using namespace TS_ENGINE;

EditorCamera::EditorCamera(const std::string& name) : Camera(name)
{
	mName = name;
	mNode->SetName(name + " Node");
	mCameraType = TS_ENGINE::Camera::Type::EDITORCAMERA;

	{

	//mSkyboxShader = TS_ENGINE::Shader::Create("HDRLighting", "HDRLighting.vert", "HDRLighting.frag");
	//mSkyboxMat = CreateRef<TS_ENGINE::Material>("SkyboxMaterial", mSkyboxShader);//Create default material

	//mCurrentShader = mSkyboxShader;

	//Create skybox gameobject	
	//Ref<Sphere> skyboxSphere = CreateRef<TS_ENGINE::Sphere>("Skybox");
	//skyboxSphere->SetMaterial(mSkyboxMat);
	//skyboxSphere->SetColor(1.0f, 1.0f, 1.0f);
	//skyboxSphere->SetTexture("industrial_sunset_puresky.jpg");
	//skyboxSphere->Create();

	//Create skybox node
	//mSkyboxNode = CreateRef<TS_ENGINE::Node>();
	//mSkyboxNode->SetName("Skybox");
	//mSkyboxNode->GetTransform()->SetLocalEulerAngles(-90.0f, 180.0f, 90.0f);
	//mSkyboxNode->GetTransform()->SetLocalScale(10000.0f, 10000.0f, 10000.0f);
	//mSkyboxNode->AttachObject(skyboxSphere);
	}
}

void EditorCamera::Initialize()
{
	mEntityID = EntityManager::GetInstance()->Instantiate(mName, mEntityType);
}

void EditorCamera::Update(Ref<TS_ENGINE::Shader> shader, float deltaTime)
{
	mNode->InitializeTransformMatrices();

	mViewMatrix = mNode->GetTransform()->GetTransformationMatrix();;
	mViewMatrix = glm::inverse(mViewMatrix);

	Controls(deltaTime);

	shader->SetVec3("u_ViewPos", mNode->GetTransform()->GetLocalPosition());
	shader->SetMat4("u_View", mViewMatrix);
	shader->SetMat4("u_Projection", mProjectionMatrix);
}

void EditorCamera::RenderGui(Ref<TS_ENGINE::Shader> shader, float deltaTime)
{

}

void EditorCamera::DeleteMeshes()
{
}

