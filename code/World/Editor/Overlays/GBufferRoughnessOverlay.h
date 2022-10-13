#pragma once

#include "Resource/Proxy.h"
#include "World/Editor/IDebugOverlay.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Shader;

}

namespace traktor::world
{

class T_DLLCLASS GBufferRoughnessOverlay : public IDebugOverlay
{
	T_RTTI_CLASS;

public:
	virtual bool create(resource::IResourceManager* resourceManager) override final;

	virtual void setup(render::RenderGraph& renderGraph, render::ScreenRenderer* screenRenderer, IWorldRenderer* worldRenderer, const WorldRenderView& worldRenderView, float alpha) const override final;

private:
	resource::Proxy< render::Shader > m_shader;
};

}
