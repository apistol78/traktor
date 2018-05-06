#ifndef traktor_amalgam_PrefabMerge_H
#define traktor_amalgam_PrefabMerge_H

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Io/Path.h"
#include "Core/Math/Transform.h"
#include "Core/Serialization/ISerializable.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Physics/Editor/MeshAsset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class T_DLLCLASS PrefabMerge : public ISerializable
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

	PrefabMerge(bool partitionMesh = false);

	void setName(const std::wstring& name);

	const std::wstring& getName() const { return m_name; }

	bool partitionMesh() const { return m_partitionMesh; }

	void addVisualMesh(const mesh::MeshAsset* visualMeshAsset, const Transform& transform);

	void addShapeMesh(const physics::MeshAsset* shapeMeshAsset, const Transform& transform);

	const AlignedVector< VisualMesh >& getVisualMeshes() const { return m_visualMeshes; }

	const AlignedVector< ShapeMesh >& getShapeMeshes() const { return m_shapeMeshes; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_name;
	bool m_partitionMesh;
	AlignedVector< VisualMesh > m_visualMeshes;
	AlignedVector< ShapeMesh > m_shapeMeshes;
};

	}
}

#endif	// traktor_amalgam_PrefabMerge_H
