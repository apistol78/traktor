#pragma once

#include "Core/Object.h"
#include "Core/Math/Vector4.h"

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

/*! \brief Spherical harmonics function.
 * \ingroup Render
 */
class T_DLLCLASS SHFunction : public Object
{
	T_RTTI_CLASS;

public:
	virtual Vector4 evaluate(float phi, float theta, const Vector4& unit) const = 0;
};

	}
}
