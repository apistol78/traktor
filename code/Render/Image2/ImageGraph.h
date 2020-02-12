#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Vector4.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"

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

class ImageGraphContext;
class ImagePass;
class ImageStep;
class ImageTargetSet;
class ITexture;
class RenderGraph;
class RenderPass;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageGraph : public Object
{
    T_RTTI_CLASS;

public:
    void addTargetSets(RenderGraph* renderGraph) const;

    void addPasses(RenderGraph* renderGraph, RenderPass* parentPass, const ImageGraphContext& cx) const;

	void setFloatParameter(handle_t handle, float value);

	void setVectorParameter(handle_t handle, const Vector4& value);

	void setTextureParameter(handle_t handle, const resource::Proxy< ITexture >& value);

private:
    friend class ImageGraphData;

    RefArray< const ImageTargetSet > m_targetSets;
    RefArray< const ImagePass > m_passes;
    RefArray< const ImageStep > m_steps;
	SmallMap< handle_t, float > m_scalarParameters;
	SmallMap< handle_t, Vector4 > m_vectorParameters;
	SmallMap< handle_t, resource::Proxy< ITexture > > m_textureParameters;
};

    }
}