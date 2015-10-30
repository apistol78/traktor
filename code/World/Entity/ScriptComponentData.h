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

	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class Entity;
class ScriptComponent;

/*! \brief Script entity component data.
 * \ingroup World
 */
class T_DLLCLASS ScriptComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< ScriptComponent > createComponent(Entity* owner, resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

private:
	friend class EntityComponentPipeline;

	resource::Id< IRuntimeClass > m_class;
};

	}
}

#endif	// traktor_world_ScriptComponentData_H
