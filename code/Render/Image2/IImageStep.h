#pragma once

#include "Core/Object.h"
#include "Core/Containers/StaticVector.h"
#include "Render/Types.h"

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
class RenderGraph;

/*!
 * \ingroup Render
 */
class T_DLLCLASS IImageStep : public Object
{
	T_RTTI_CLASS;

public:
	typedef StaticVector< handle_t, 32 > targetSetVector_t;

	virtual void addPasses(const ImageGraph* graph, const ImageGraphContext& context, const targetSetVector_t& targetSetIds, RenderGraph& renderGraph) const = 0;
};

	}
}