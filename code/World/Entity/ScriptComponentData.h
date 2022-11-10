/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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
class PropertyGroup;

	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class ScriptComponent;

/*! Script entity component data.
 * \ingroup World
 */
class T_DLLCLASS ScriptComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	ScriptComponentData() = default;

	explicit ScriptComponentData(const resource::Id< IRuntimeClass >& _class);

	Ref< ScriptComponent > createComponent(resource::IResourceManager* resourceManager) const;

	virtual void setTransform(const EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< IRuntimeClass >& getRuntimeClass() const { return m_class;  }

	bool getEditorSupport() const { return m_editorSupport; }

private:
	resource::Id< IRuntimeClass > m_class;
	Ref< const PropertyGroup > m_properties;
	bool m_editorSupport = false;
};

	}
}

