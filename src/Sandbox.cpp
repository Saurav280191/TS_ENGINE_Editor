#include <TS_ENGINE.h>
#include <Core/EntryPoint.h>
#include "SandboxLayer.h"

class Sandbox : public TS_ENGINE::Application
{
public:
	Sandbox()
	{
		PushLayer(new SandboxLayer());
	}
};

TS_ENGINE::Application* TS_ENGINE::CreateApplication()
{
	return new Sandbox();
}
