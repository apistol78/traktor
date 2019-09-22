#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace render
	{

class ICubeTexture;
class IRenderSystem;
class IRenderView;
class RenderTargetSet;

	}

	namespace resource
	{
	
class IResourceManager;

	}

	namespace world
	{

class IWorldRenderer;
class WorldEntityRenderers;
	
class ProbeCapturer : public Object
{
	T_RTTI_CLASS;

public:
	ProbeCapturer(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	bool create();

	void build(
		WorldEntityRenderers* entityRenderers,
		Entity* rootEntity,
		const Vector4& pivot
	);

	void render(render::IRenderView* renderView);

	void transfer(render::ICubeTexture* probeTexture);

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< IWorldRenderer > m_worldRenderer;
	Ref< render::RenderTargetSet > m_renderTargetSet;
	mutable AlignedVector< uint8_t > m_data;
};

	}
}