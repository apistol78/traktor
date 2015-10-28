#ifndef traktor_world_ScriptComponentData_H
#define traktor_world_ScriptComponentData_H

#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;

	namespace world
	{

/*! \brief Script entity component data.
 * \ingroup World
 */
class T_DLLCLASS ScriptComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	virtual Ref< IEntityComponent > createInstance(Entity* owner, resource::IResourceManager* resourceManager) const T_FINAL;

	virtual void serialize(ISerializer& s);

private:
	friend class EntityComponentPipeline;

	resource::Id< IRuntimeClass > m_class;
};

	}
}

#endif	// traktor_world_ScriptComponentData_H
