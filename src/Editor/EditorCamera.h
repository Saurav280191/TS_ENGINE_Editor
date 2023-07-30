#pragma once
#include "Core/tspch.h"
#include "Renderer/Camera.h"
#include "Primitive/Sphere.h"
#include "SceneManager/Node.h"
#include "Core/Transform.h"

class EditorCamera : public TS_ENGINE::Camera
{
public:
	EditorCamera(const std::string& name);
	// Inherited via Camera
	virtual void Initialize() override;
	virtual void SetName(const std::string& name) override;
	virtual void Update(float deltaTime) override;
private:
	Ref<TS_ENGINE::Node> mSkyboxNode;
	Ref<TS_ENGINE::Shader> mSkyboxShader;
	Ref<TS_ENGINE::Material> mSkyboxMat;

};
