/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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
class PropertyGroup;

}

namespace traktor::world
{

/*! Script component.
 * \ingroup World
 */
class T_DLLCLASS ScriptComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit ScriptComponent(const resource::Proxy< IRuntimeClass >& clazz, const PropertyGroup* properties);

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const UpdateParams& update) override final;

	void execute(const char* method);

	Entity* getOwner() const { return m_owner; }

	const PropertyGroup* getProperties() const { return m_properties; }

private:
	Entity* m_owner;
	resource::Proxy< IRuntimeClass > m_class;
	Ref< const PropertyGroup > m_properties;
	Ref< ITypedObject > m_object;
	Ref< const IRuntimeDispatch > m_methodSetTransform;
	Ref< const IRuntimeDispatch > m_methodUpdate;

	bool validate();
};

}
