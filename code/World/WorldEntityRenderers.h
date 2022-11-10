/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IEntityRenderer;

/*! World entity renderers.
 * \ingroup World
 */
class T_DLLCLASS WorldEntityRenderers : public Object
{
	T_RTTI_CLASS;

public:
	typedef SmallMap< const TypeInfo*, IEntityRenderer* > entity_renderer_map_t;

	void add(IEntityRenderer* entityRenderer);

	void remove(IEntityRenderer* entityRenderer);

	IEntityRenderer* find(const TypeInfo& entityType) const {
		const auto it = m_entityRendererMap.find(&entityType);
		return it != m_entityRendererMap.end() ? it->second : nullptr;		
	}

	const RefArray< IEntityRenderer >& get() const { return m_entityRenderers; }

private:
	RefArray< IEntityRenderer > m_entityRenderers;
	entity_renderer_map_t m_entityRendererMap;
};

}
