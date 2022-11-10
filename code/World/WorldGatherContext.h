/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class Entity;
class IEntityRenderer;
class WorldEntityRenderers;

/*! World gather context.
 * \ingroup World
 */
class T_DLLCLASS WorldGatherContext : public Object
{
	T_RTTI_CLASS;

public:
	typedef std::function< void(IEntityRenderer*, Object*) > gatherFn_t;

	explicit WorldGatherContext(const WorldEntityRenderers* entityRenderers, const Entity* rootEntity, const gatherFn_t& filter);

	void gather(Object* renderable) const;

	void include(IEntityRenderer* entityRenderer, Object* renderable) const;

	const Entity* getRootEntity() const { return m_rootEntity; }

private:
	const WorldEntityRenderers* m_entityRenderers;
	const Entity* m_rootEntity;
	gatherFn_t m_filter;
};

}
