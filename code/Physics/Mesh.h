#ifndef traktor_physics_Mesh_H
#define traktor_physics_Mesh_H

#include "Core/Object.h"
#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace physics
	{

/*! \brief Mesh geometry.
 * \ingroup Physics
 */
class T_DLLCLASS Mesh : public Object
{
	T_RTTI_CLASS;

public:
#pragma pack(1)
	struct Triangle
	{
		uint32_t indices[3];
	};
#pragma pack()

	Mesh();

	void setVertices(const AlignedVector< Vector4 >& vertices);

	const AlignedVector< Vector4 >& getVertices() const;

	void setShapeTriangles(const std::vector< Triangle >& shapeTriangles);

	const std::vector< Triangle >& getShapeTriangles() const;

	void setHullTriangles(const std::vector< Triangle >& hullTriangles);

	const std::vector< Triangle >& getHullTriangles() const;

	void setOffset(const Vector4& offset);

	const Vector4& getOffset() const;

	void setMargin(float margin);

	float getMargin() const;

	bool read(IStream* stream);

	bool write(IStream* stream);

private:
	AlignedVector< Vector4 > m_vertices;
	std::vector< Triangle > m_shapeTriangles;
	std::vector< Triangle > m_hullTriangles;
	Vector4 m_offset;
	float m_margin;
};

	}
}

#endif	// traktor_physics_Mesh_H
