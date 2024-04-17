/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/AbstractEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::ai
{

/*! Navigation mesh entity factory.
 * \ingroup AI
 */
class T_DLLCLASS NavMeshEntityFactory : public world::AbstractEntityFactory
{
	T_RTTI_CLASS;

public:
	explicit NavMeshEntityFactory(resource::IResourceManager* resourceManager, bool suppress);

	virtual const TypeInfoSet getEntityComponentTypes() const override final;

	virtual Ref< world::IEntityComponent > createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const override final;

private:
	Ref< resource::IResourceManager > m_resourceManager;
	bool m_suppress;
};

}
