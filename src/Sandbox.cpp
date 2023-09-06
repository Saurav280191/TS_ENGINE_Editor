#include <TS_ENGINE.h>
#include <Core/EntryPoint.h>
#include "EditorLayer.h"

class Sandbox : public TS_ENGINE::Application
{
public:
	Sandbox()
	{
		PushLayer(new EditorLayer());
	}
};

TS_ENGINE::Application* TS_ENGINE::CreateApplication(std::filesystem::path exeDir)
{
	Application::SetExecutableDirectory(exeDir);
	return new Sandbox();
}
