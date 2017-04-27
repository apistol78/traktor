/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_PointSet_H
#define traktor_spray_PointSet_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace spray
	{

/*! \brief Point set
 * \ingroup Spray
 */
class T_DLLCLASS PointSet : public Object
{
	T_RTTI_CLASS;

public:
	struct Point
	{
		Vector4 position;
		Vector4 normal;
		Vector4 color;
	};

	PointSet();

	void add(const Point& point);

	const AlignedVector< Point >& get() const { return m_points; }
	
	bool read(IStream* stream);
	
	bool write(IStream* stream) const;

private:
	AlignedVector< Point > m_points;
};

	}
}

#endif	// traktor_spray_PointSet_H
