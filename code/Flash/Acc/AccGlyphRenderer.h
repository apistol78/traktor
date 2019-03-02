#pragma once

#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class RenderContext;
class RenderTargetSet;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace flash
	{

class AccGlyphRenderer : public Object
{
	T_RTTI_CLASS;

public:
	bool create(render::IRenderSystem* renderSystem, resource::IResourceManager* resourceManager);

	void destroy();

private:
	Ref< render::RenderTargetSet > m_renderTargetGlyphs;
};

	}
}

