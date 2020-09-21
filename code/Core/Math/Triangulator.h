#pragma once

#include <functional>
#include "Core/Config.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Triangulator.
 * \ingroup Core
 */
class T_DLLCLASS Triangulator
{
public:
	enum TriangulatorFlags
	{
		TfSequential = 0,
		TfSorted = 1
	};

	typedef std::function< void(size_t index0, size_t index1, size_t index2) > fn_callback_t;

	void freeze(
		const AlignedVector< Vector2 >& points,
		uint32_t flags,
		const fn_callback_t& callback
	);

	void freeze(
		const AlignedVector< Vector4 >& points,
		const Vector4& normal,
		uint32_t flags,
		const fn_callback_t& callback
	);
};

}

