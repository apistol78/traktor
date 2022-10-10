#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*! Vertex layout. 
 * \ingroup Render
 */
class T_DLLCLASS IVertexLayout : public Object
{
	T_RTTI_CLASS;
};

}
