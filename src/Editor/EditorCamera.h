#pragma once
#include <Core/tspch.h>
#include <Renderer/Camera/Camera.h>
#include <Primitive/Sphere.h>
#include <SceneManager/Node.h>
#include <Core/Transform.h>

class EditorCamera : public TS_ENGINE::Camera
{
public:
	EditorCamera(const std::string& name);

	// Inherited via Camera	
	virtual void Initialize() override;
	virtual void Update(Ref<TS_ENGINE::Shader> shader, float deltaTime) override;	
	virtual void RenderGui(Ref<TS_ENGINE::Shader> shader, float deltaTime) override;
	virtual void DeleteMeshes() override;
private:
	Ref<TS_ENGINE::Node> mSkyboxNode;
	Ref<TS_ENGINE::Shader> mSkyboxShader;
	Ref<TS_ENGINE::Material> mSkyboxMat;
};
