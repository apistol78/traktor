#ifndef traktor_world_ScriptEntityComponent_H
#define traktor_world_ScriptEntityComponent_H

#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

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

class Entity;

/*! \brief Script entity component.
 * \ingroup World
 */
class T_DLLCLASS ScriptEntityComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	ScriptEntityComponent(Entity* owner, const resource::Proxy< IRuntimeClass >& clazz);

	virtual void update(const UpdateParams& update) T_FINAL;

private:
	Entity* m_owner;
	resource::Proxy< IRuntimeClass > m_class;
	Ref< ITypedObject > m_object;
	uint32_t m_methodUpdate;
};

	}
}

#endif	// traktor_world_ScriptEntityComponent_H
