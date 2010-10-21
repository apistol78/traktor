#ifndef traktor_mesh_InstanceMeshData_H
#define traktor_mesh_InstanceMeshData_H

#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Transform.h"

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
	float rotation[4];
	float translation[3];
	float parameter;
};
#pragma pack()

inline InstanceMeshData packInstanceMeshData(const Transform& transform, float userParameter)
{
	InstanceMeshData T_MATH_ALIGN16 imd;

	transform.rotation().e.storeAligned(imd.rotation);
	transform.translation().storeAligned(imd.translation);
	imd.parameter = userParameter;

	return imd;
}

	}
}

#endif	// traktor_mesh_InstanceMeshData_H
