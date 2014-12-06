#ifndef traktor_ai_NavMeshEntityData_H
#define traktor_ai_NavMeshEntityData_H

#include "Resource/Id.h"
#include "World/EntityData.h"

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
class T_DLLCLASS NavMeshEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	const resource::Id< NavMesh >& get() const { return m_navMesh; }

	virtual void serialize(ISerializer& s);

private:
	resource::Id< NavMesh > m_navMesh;
};

	}
}

#endif	// traktor_ai_NavMeshEntityData_H
