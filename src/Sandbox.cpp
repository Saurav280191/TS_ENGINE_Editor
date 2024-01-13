#include <TS_ENGINE.h>
#include <Core/EntryPoint.h>
#include "EditorLayer.h"
#include "Renderer/MaterialManager.h"

class Sandbox : public TS_ENGINE::Application
{
public:
	Sandbox()
	{
		TS_ENGINE::MaterialManager::GetInstance()->LoadAllShadersAndCreateMaterials();
		PushLayer(new EditorLayer());
	}
};

TS_ENGINE::Application* TS_ENGINE::CreateApplication(std::filesystem::path exeDir)
{
	Application::SetExecutableDirectory(exeDir);
	return new Sandbox();
}
