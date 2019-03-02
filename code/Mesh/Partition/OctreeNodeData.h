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

namespace traktor
{
	namespace mesh
	{

class T_DLLCLASS OctreeNodeData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

//private:
	Aabb3 m_boundingBox;
	std::map< uint8_t, std::vector< uint32_t > > m_partIndices;
	Ref< OctreeNodeData > m_children[8];
};

	}
}

