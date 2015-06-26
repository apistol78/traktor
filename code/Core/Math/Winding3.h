#ifndef traktor_Winding3_H
#define traktor_Winding3_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Plane.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Winding2;

/*! \brief 3d point winding.
 * \ingroup Core
 */
class T_DLLCLASS Winding3
{
public:
	typedef AlignedVector< Vector4 > points_t;

	enum Classification
	{
		CfFront,
		CfBack,
		CfCoplanar,
		CfSpan
	};

	Winding3();

	Winding3(const points_t& points);

	Winding3(const Vector4* points, size_t npoints);

	Winding3(const Vector4& p1, const Vector4& p2, const Vector4& p3);

	void clear();

	void push(const Vector4& p);

	bool angleIndices(uint32_t& outI1, uint32_t& outI2, uint32_t& outI3) const;

	bool getProjection(Winding2& outProjection, Vector4& outU, Vector4& outV) const;

	bool getPlane(Plane& outPlane) const;

	void split(const Plane& plane, Winding3& outFront, Winding3& outBack) const;

	int classify(const Plane& plane) const;

	float area() const;

	Vector4 center() const;
	
	bool rayIntersection(
		const Vector4& origin,
		const Vector4& direction,
		Scalar& outK,
		Vector4* outPoint = 0
	) const;

	void flip();

	const points_t& getPoints() const { return m_points; }

	uint32_t size() const { return uint32_t(m_points.size()); }

	bool empty() const { return m_points.empty(); }

	const Vector4& operator [] (uint32_t index) const { return m_points[index]; }

private:
	points_t m_points;
};

}

#endif	// traktor_Winding3_H
