#pragma once

#include <list>
#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

/*! \brief
 */
class LodMeshAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	LodMeshAsset();

	virtual void serialize(ISerializer& s) override final;

private:
	friend class LodMeshPipeline;

	float m_maxDistance;
	float m_cullDistance;
	std::list< Guid > m_lods;
};

	}
}

