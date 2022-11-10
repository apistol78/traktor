#pragma once

#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Transform.h"

namespace traktor::mesh
{

/*!
 * Per instance data structure.
 *
 * \note
 * Must be packed into Vector4;s as an array
 * of this structure is feed directly as
 * shader parameters.
 */
#pragma pack(1)
struct T_MATH_ALIGN16 InstanceMeshData
{
	float rotation[4];
	float translation[3];
	float scale;
};
#pragma pack()

inline InstanceMeshData packInstanceMeshData(const Transform& transform)
{
	InstanceMeshData T_MATH_ALIGN16 imd;
	transform.rotation().e.storeAligned(imd.rotation);
	transform.translation().storeAligned(imd.translation);
	imd.scale = 1.0f;
	return imd;
}

}
