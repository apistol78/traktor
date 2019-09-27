#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SVG_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace svg
	{

class Shape;

/*! SVG document visitor interface.
 * \ingroup SVG
 */
class T_DLLCLASS IShapeVisitor : public Object
{
	T_RTTI_CLASS;

public:
	virtual void enter(Shape* shape) = 0;

	virtual void leave(Shape* shape) = 0;
};

	}
}
