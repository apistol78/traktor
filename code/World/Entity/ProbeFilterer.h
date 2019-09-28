#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Resource/Proxy.h"

namespace traktor
{
    namespace render
    {

class ICubeTexture;
class IRenderSystem;
class IRenderView;
class RenderTargetSet;
class ScreenRenderer;

    }

    namespace resource
    {

class IResourceManager;

    }

    namespace world
    {

class ProbeFilterer : public Object
{
    T_RTTI_CLASS;

public:
	ProbeFilterer(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	bool create();

	void render(render::IRenderView* renderView, render::ICubeTexture* probeTexture);

private:
    Ref< resource::IResourceManager > m_resourceManager;
    Ref< render::IRenderSystem > m_renderSystem;
    resource::Proxy< render::Shader > m_filterShader;
    Ref< render::ScreenRenderer > m_screenRenderer;
    RefArray< render::RenderTargetSet > m_renderTargetSets;
};

    }
}
