#ifndef traktor_mesh_InstanceMeshData_H
#define traktor_mesh_InstanceMeshData_H

#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"

namespace traktor
{
	namespace mesh
	{

/*!
 * \brief Per instance data structure.
 *
 * \note
 * Must be packed into Vector4;s as an array
 * of this structure is feed directly as
 * shader parameters.
 */
#pragma pack(1)
struct InstanceMeshData
{
	Quaternion rotation;
	Vector4 translation;
};
#pragma pack()

inline InstanceMeshData packInstanceMeshData(const Matrix44& transform)
{
	InstanceMeshData imd;
	imd.rotation = Quaternion(transform).normalized();
	imd.translation = transform.translation();
	return imd;
}

	}
}

#endif	// traktor_mesh_InstanceMeshData_H
