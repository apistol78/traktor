/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Mesh/Editor/MeshEntityEditor.h"
#include "Scene/Editor/EntityAdapter.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshEntityEditor", MeshEntityEditor, scene::DefaultEntityEditor)

MeshEntityEditor::MeshEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}

bool MeshEntityEditor::isPickable() const
{
	return true;
}

bool MeshEntityEditor::queryRay(
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection,
	Scalar& outDistance
) const
{
	// Transform ray into object space.
	Transform worldInv = getEntityAdapter()->getTransform().inverse();
	Vector4 objectRayOrigin = worldInv * worldRayOrigin;
	Vector4 objectRayDirection = worldInv * worldRayDirection;

	// Get entity bounding box; do not pick if origin of ray is within box.
	Aabb3 boundingBox = getEntityAdapter()->getBoundingBox();
	if (boundingBox.empty() || boundingBox.inside(objectRayOrigin))
		return false;

	// Trace bounding box to see if ray intersect.
	Scalar length = outDistance - Scalar(FUZZY_EPSILON);
	Scalar distance;
	if (!boundingBox.intersectSegment(objectRayOrigin, objectRayOrigin + objectRayDirection * length, distance))
		return false;

	T_FATAL_ASSERT (distance <= 1.0f);
	outDistance = distance * length;
	return true;
}

	}
}
