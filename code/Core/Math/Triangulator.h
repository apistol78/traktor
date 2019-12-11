#pragma once

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

	struct Triangle
	{
		size_t indices[3];
	};

	void freeze(
		const AlignedVector< Vector2 >& points,
		AlignedVector< Triangle >& outTriangles,
		uint32_t flags = TfSorted
	);

	void freeze(
		const AlignedVector< Vector4 >& points,
		const Vector4& normal,
		AlignedVector< Triangle >& outTriangles,
		uint32_t flags = TfSorted
	);
};

}

