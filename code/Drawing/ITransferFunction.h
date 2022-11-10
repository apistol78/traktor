#pragma once

#include "Core/Object.h"
#include "Core/Math/Color4f.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

class Image;

/*! Image copy transfer function base class.
 * \ingroup Drawing
 */
class T_DLLCLASS ITransferFunction : public Object
{
	T_RTTI_CLASS;

protected:
	friend class Image;

	virtual void evaluate(const Color4f& in, Color4f& out) const = 0;
};

}
