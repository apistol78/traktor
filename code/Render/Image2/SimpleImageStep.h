#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"
#include "Render/Image2/IImageStep.h"
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

class Shader;

/*!
 * \ingroup Render
 */
class T_DLLCLASS SimpleImageStep : public IImageStep
{
	T_RTTI_CLASS;

public:
	virtual void setup(const ImageGraph* imageGraph, RenderPass& pass) const override final;

    virtual void build(
        const ImageGraph* imageGraph,
        const RenderGraph& renderGraph,
        RenderContext* renderContext,
        const ImageGraphParams& data
	) const override final;

private:
	friend class SimpleImageStepData;

	struct Source
	{
		handle_t parameter;
		handle_t targetSet;
		int32_t colorIndex;
	};

	resource::Proxy< render::Shader > m_shader;
	AlignedVector< Source > m_sources;
};

	}
}