#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace render
    {

class IImageStep;
class ImagePass;
class IRenderSystem;
class RenderContext;
class RenderGraph;
class RenderPass;
class ScreenRenderer;

/*!
 * \ingroup Render
 */
struct ImageGraphParams
{
    Frustum viewFrustum;
    Matrix44 lastView;
    Matrix44 view;
    Matrix44 viewToLight;
    Matrix44 projection;
    Vector4 godRayDirection;
    Vector4 shadowMapUvTransform;
    int32_t sliceCount;
    int32_t sliceIndex;
    float sliceNearZ;
    float sliceFarZ;
    float shadowFarZ;
    float shadowMapBias;
    float deltaTime;
    float time;

    ImageGraphParams()
    :	shadowMapUvTransform(0.0f, 0.0f, 1.0f, 1.0f)
    ,	sliceCount(0)
    ,	sliceIndex(0)
    ,	sliceNearZ(0.0f)
    ,	sliceFarZ(0.0f)
    ,	shadowFarZ(0.0f)
    ,	shadowMapBias(0.0f)
    ,	deltaTime(0.0f)
    ,   time(0.0f)
    {
    }
};

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageGraph : public Object
{
    T_RTTI_CLASS;

public:
    bool create(IRenderSystem* renderSystem, RenderGraph* renderGraph);

    void destroy();

    void addPasses(RenderGraph* renderGraph, RenderPass* parentPass, const ImageGraphParams& data) const;

    ScreenRenderer* getScreenRenderer() const { return m_screenRenderer; }

private:
    friend class ImageGraphData;

    Ref< ScreenRenderer > m_screenRenderer;
    RefArray< const ImagePass > m_passes;
    RefArray< const IImageStep > m_steps;
};

    }
}