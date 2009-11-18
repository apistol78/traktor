#ifndef traktor_spray_PointSet_H
#define traktor_spray_PointSet_H

#include "Core/Serialization/ISerializable.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

/*! \brief Point set
 * \ingroup Spray
 */
class T_DLLCLASS PointSet : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Point
	{
		Vector4 position;
		Vector4 normal;
		Vector4 color;

		bool serialize(ISerializer& s);
	};

	PointSet();

	PointSet(const AlignedVector< Point >& points);

	const AlignedVector< Point >& getPoints() const { return m_points; }

	virtual bool serialize(ISerializer& s);

private:
	AlignedVector< Point > m_points;
};

	}
}

#endif	// traktor_spray_PointSet_H
