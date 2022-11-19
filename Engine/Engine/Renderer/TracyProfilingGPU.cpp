#include "pch.h"
#include "TracyProfilingGPU.h"

tracy::D3D11Ctx* globalTracyContext = nullptr;

void Engine::CreateTracyGPUProfiling(ID3D11Device* aDevice, ID3D11DeviceContext* aContext)
{
	globalTracyContext = TracyD3D11Context(aDevice, aContext);
}

void Engine::DestroyTracyGPUProfiling()
{
	TracyD3D11Destroy(globalTracyContext);
}

void Engine::CollectTracyGPUProfiling()
{
	TracyD3D11Collect(globalTracyContext);
}
