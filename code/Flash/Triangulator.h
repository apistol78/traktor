/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_Triangulator_H
#define traktor_flash_Triangulator_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Flash/Polygon.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief Trapezoid triangulation.
 * \ingroup Flash
 */
class T_DLLCLASS Triangulator : public Object
{
	T_RTTI_CLASS;

public:
	void triangulate(const AlignedVector< Segment >& segments, uint16_t currentFillStyle, bool oddEven, AlignedVector< Triangle >& outTriangles);

private:
	AlignedVector< Segment > m_segments;
	AlignedVector< Segment > m_slabs;
};

	}
}

#endif	// traktor_flash_Triangulator_H
