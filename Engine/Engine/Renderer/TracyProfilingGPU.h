#pragma once

#define GPU_ZONE(name) TracyD3D11Zone(globalTracyContext, name)
#define GPU_ZONE_NAMED(varName, name) TracyD3D11NamedZone(globalTracyContext, varname, name, true)
#define CPU_ZONE(name) ZoneScopedN("Scene::CullEverything")
#define CPU_ZONE_NAMED(varName, name) ZoneNamedN(varName, name, true)

namespace tracy
{
	class D3D11Ctx;
}

extern tracy::D3D11Ctx* globalTracyContext;

namespace Engine
{
	void CreateTracyGPUProfiling(ID3D11Device* aDevice, ID3D11DeviceContext* aContext);
	void DestroyTracyGPUProfiling();
	void CollectTracyGPUProfiling();
}