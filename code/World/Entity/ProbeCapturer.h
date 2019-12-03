#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace render
	{

class ICubeTexture;
class IRenderSystem;
class IRenderTargetSet;
class IRenderView;

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
	ProbeCapturer(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const TypeInfo& worldRendererType);

	bool create();

	void build(
		WorldEntityRenderers* entityRenderers,
		Entity* rootEntity,
		const Vector4& pivot,
		int32_t face
	);

	void render(render::IRenderView* renderView, render::ICubeTexture* probeTexture, int32_t face);

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	const TypeInfo& m_worldRendererType;
	Ref< IWorldRenderer > m_worldRenderer;
	Ref< render::IRenderTargetSet > m_renderTargetSet;
	mutable AlignedVector< uint8_t > m_data;
};

	}
}