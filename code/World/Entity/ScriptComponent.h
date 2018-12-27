/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_ScriptComponent_H
#define traktor_world_ScriptComponent_H

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
class IRuntimeDispatch;

	namespace world
	{

class Entity;

/*! \brief Script component.
 * \ingroup World
 */
class T_DLLCLASS ScriptComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	ScriptComponent(const resource::Proxy< IRuntimeClass >& clazz);

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const UpdateParams& update) override final;

private:
	Entity* m_owner;
	resource::Proxy< IRuntimeClass > m_class;
	Ref< ITypedObject > m_object;
	Ref< const IRuntimeDispatch > m_methodUpdate;
};

	}
}

#endif	// traktor_world_ScriptComponent_H
