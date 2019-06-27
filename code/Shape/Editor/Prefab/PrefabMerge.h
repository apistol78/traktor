#pragma once

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Transform.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class MeshAsset;

	}

	namespace physics
	{

class MeshAsset;

	}

	namespace shape
	{

class T_DLLCLASS PrefabMerge : public Object
{
	T_RTTI_CLASS;

public:
	struct VisualMesh
	{
		Ref< const mesh::MeshAsset > meshAsset;
		Transform transform;

		void serialize(ISerializer& s);
	};

	struct ShapeMesh
	{
		Ref< const physics::MeshAsset > meshAsset;
		Transform transform;

		void serialize(ISerializer& s);
	};

	void addVisualMesh(const mesh::MeshAsset* visualMeshAsset, const Transform& transform);

	void addShapeMesh(const physics::MeshAsset* shapeMeshAsset, const Transform& transform);

	const AlignedVector< VisualMesh >& getVisualMeshes() const { return m_visualMeshes; }

	const AlignedVector< ShapeMesh >& getShapeMeshes() const { return m_shapeMeshes; }

private:
	AlignedVector< VisualMesh > m_visualMeshes;
	AlignedVector< ShapeMesh > m_shapeMeshes;
};

	}
}

