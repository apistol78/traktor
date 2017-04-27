/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Triangulator_H
#define traktor_Triangulator_H

#include <vector>
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

/*! \brief Triangulator.
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
		std::vector< Triangle >& outTriangles,
		uint32_t flags = TfSorted
	);

	void freeze(
		const AlignedVector< Vector4 >& points,
		const Vector4& normal,
		std::vector< Triangle >& outTriangles,
		uint32_t flags = TfSorted
	);
};

}

#endif	// traktor_Triangulator_H
