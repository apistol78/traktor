#include "Render/ICubeTexture.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "World/Entity/ProbeFilterer.h"

namespace traktor
{
    namespace world
    {
        namespace
        {

#if !defined(__ANDROID__)
const int32_t c_faceSize = 512;
#else
const int32_t c_faceSize = 128;
#endif

const resource::Id< render::Shader > c_idFilterShader(Guid(L"{D9CC2267-0BDF-4A19-A970-856112821734}"));

        }

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ProbeFilterer", ProbeFilterer, Object)

ProbeFilterer::ProbeFilterer(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:   m_resourceManager(resourceManager)
,   m_renderSystem(renderSystem)
{
}

bool ProbeFilterer::create()
{
    if (!m_resourceManager->bind(c_idFilterShader, m_filterShader))
        return false;

    m_screenRenderer = new render::ScreenRenderer();
    if (!m_screenRenderer->create(m_renderSystem))
        return false;

    const int32_t mipCount = log2(c_faceSize) + 1;
    m_renderTargetSets.resize(mipCount);

    for (int32_t mip = 1; mip < mipCount; ++mip)
    {
        render::RenderTargetSetCreateDesc rtscd;
        rtscd.count = 1;
        rtscd.width = c_faceSize >> mip;
        rtscd.height = c_faceSize >> mip;
        rtscd.multiSample = 0;
        rtscd.createDepthStencil = false;
        rtscd.usingDepthStencilAsTexture = false;
        rtscd.usingPrimaryDepthStencil = false;
        rtscd.preferTiled = false;
        rtscd.ignoreStencil = true;
        rtscd.generateMips = false;
        rtscd.targets[0].format = render::TfR11G11B10F;
        rtscd.targets[0].sRGB = false;

        m_renderTargetSets[mip] = m_renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
        if (!m_renderTargetSets[mip])
            return false;
    }

    return true;
}

void ProbeFilterer::render(render::IRenderView* renderView, render::ICubeTexture* probeTexture)
{
    const int32_t mipCount = log2(c_faceSize) + 1;

	render::Clear clear = { 0 };
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
    
    m_filterShader->setTextureParameter(L"World_ProbeTexture", probeTexture);

    for (int32_t side = 0; side < 6; ++side)
    {
        Vector4 corners[4];

        switch (side)
        {
        case 0:
        case 1:
            corners[1].set(side == 0 ? 1.0f : -1.0f, -1.0f, -1.0f, 0.0f);
            corners[0].set(side == 0 ? 1.0f : -1.0f,  1.0f, -1.0f, 0.0f);
            corners[3].set(side == 0 ? 1.0f : -1.0f, -1.0f,  1.0f, 0.0f);
            corners[2].set(side == 0 ? 1.0f : -1.0f,  1.0f,  1.0f, 0.0f);
            break;

        case 2:
        case 3:
            corners[0].set(-1.0f, side == 2 ? 1.0f : -1.0f, -1.0f, 0.0f);
            corners[1].set( 1.0f, side == 2 ? 1.0f : -1.0f, -1.0f, 0.0f);
            corners[2].set(-1.0f, side == 2 ? 1.0f : -1.0f,  1.0f, 0.0f);
            corners[3].set( 1.0f, side == 2 ? 1.0f : -1.0f,  1.0f, 0.0f);
            break;

        case 4:
        case 5:
            corners[0].set(-1.0f, -1.0f, side == 4 ? 1.0f : -1.0f, 0.0f);
            corners[1].set( 1.0f, -1.0f, side == 4 ? 1.0f : -1.0f, 0.0f);
            corners[2].set(-1.0f,  1.0f, side == 4 ? 1.0f : -1.0f, 0.0f);
            corners[3].set( 1.0f,  1.0f, side == 4 ? 1.0f : -1.0f, 0.0f);
            break;
        }

        m_filterShader->setVectorArrayParameter(L"World_FilterCorners", corners, sizeof_array(corners));

        for (int32_t mip = 1; mip < mipCount; ++mip)
        {
            renderView->begin(m_renderTargetSets[mip], &clear);
            m_screenRenderer->draw(renderView, m_filterShader);
            renderView->end();
            renderView->copy(
                probeTexture,
                side,
                mip,
                m_renderTargetSets[mip]->getColorTexture(0),
                0,
                0
            );
        }
    }
}

    }
}