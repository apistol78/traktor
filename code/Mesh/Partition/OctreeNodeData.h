#pragma once

#include "Core/Math/Aabb3.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

/*!
 * \ingroup Mesh
 */
class T_DLLCLASS OctreeNodeData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setBoundingBox(const Aabb3& boundingBox);

	void addPartIndex(uint8_t worldTechniqueId, uint32_t partIndex);

	void setChild(int32_t index, const OctreeNodeData* child);

	virtual void serialize(ISerializer& s) override final;

private:
	friend class OctreeNode;

	Aabb3 m_boundingBox;
	SmallMap< uint8_t, AlignedVector< uint32_t > > m_partIndices;
	Ref< const OctreeNodeData > m_children[8];
};

}
