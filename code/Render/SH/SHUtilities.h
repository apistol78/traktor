#pragma once

#include "Core/Math/Matrix44.h"
#include "Render/SH/SHMatrix.h"

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

/*! \brief Generate rotation SH matrix.
 * \ingroup Render
 *
 * Generate SH rotation matrix from Cartesian rotation matrix.
 */
SHMatrix T_DLLCLASS generateRotationSHMatrix(const Matrix44& matrix, int order);

	}
}
