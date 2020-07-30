#pragma once

#include "Core/Object.h"
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

class ImageGraph;
class ImageGraphContext;
class ProgramParameters;
class RenderContext;
class RenderGraph;
class RenderPass;
class Shader;

/*! Image pass step.
 * \ingroup Render
 * 
 * Each pass can have a sequence of operations
 * to implement it's functionality.
 */
class T_DLLCLASS ImagePassOp : public Object
{
	T_RTTI_CLASS;

public:
	/*! */
	virtual void setup(const ImageGraph* imageGraph, const ImageGraphContext& cx, RenderPass& pass) const = 0;

	/*! */
	virtual void build(
		const ImageGraph* imageGraph,
		const ImageGraphContext& cx,
		const RenderGraph& renderGraph,
		const ProgramParameters* sharedParams,
		RenderContext* renderContext
	) const = 0;

protected:
	friend class ImagePassOpData;

	struct Source
	{
		handle_t textureId;
		handle_t parameter;
	};

	resource::Proxy< render::Shader > m_shader;
	AlignedVector< Source > m_sources;    
};

	}
}