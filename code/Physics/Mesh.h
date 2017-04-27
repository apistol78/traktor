/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	void setNormals(const AlignedVector< Vector4 >& normals);

	const AlignedVector< Vector4 >& getNormals() const;

	void setShapeTriangles(const AlignedVector< Triangle >& shapeTriangles);

	const AlignedVector< Triangle >& getShapeTriangles() const;

	void setHullTriangles(const AlignedVector< Triangle >& hullTriangles);

	const AlignedVector< Triangle >& getHullTriangles() const;

	void setHullIndices(const AlignedVector< uint32_t >& hullIndices);

	const AlignedVector< uint32_t >& getHullIndices() const;

	void setOffset(const Vector4& offset);

	const Vector4& getOffset() const;

	void setMargin(float margin);

	float getMargin() const;

	bool read(IStream* stream);

	bool write(IStream* stream);

private:
	AlignedVector< Vector4 > m_vertices;
	AlignedVector< Vector4 > m_normals;
	AlignedVector< Triangle > m_shapeTriangles;
	AlignedVector< Triangle > m_hullTriangles;
	AlignedVector< uint32_t > m_hullIndices;
	Vector4 m_offset;
	float m_margin;
};

	}
}

#endif	// traktor_physics_Mesh_H
