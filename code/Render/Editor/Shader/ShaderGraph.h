#pragma once

#include "Render/Editor/Graph.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! Shader graph.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraph : public Graph
{
	T_RTTI_CLASS;

public:
	ShaderGraph();

	ShaderGraph(const RefArray< Node >& nodes, const RefArray< Edge >& edges);
};

	}
}
