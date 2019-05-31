#pragma once

#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ai
	{

class NavMesh;

/*! \brief Navigation mesh entity data.
 * \ingroup AI
 */
class T_DLLCLASS NavMeshComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	const resource::Id< NavMesh >& get() const { return m_navMesh; }

	virtual void serialize(ISerializer& s) override final;

private:
	resource::Id< NavMesh > m_navMesh;
};

	}
}
