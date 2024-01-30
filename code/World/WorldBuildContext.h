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
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class RenderContext;

}

namespace traktor::world
{

class WorldEntityRenderers;

/*! World build context.
 * \ingroup World
 */
class T_DLLCLASS WorldBuildContext : public Object
{
	T_RTTI_CLASS;

public:
	explicit WorldBuildContext(const WorldEntityRenderers* entityRenderers, render::RenderContext* renderContext);

	const WorldEntityRenderers* getEntityRenderers() const { return m_entityRenderers; }

	render::RenderContext* getRenderContext() const { return m_renderContext; }

private:
	const WorldEntityRenderers* m_entityRenderers;
	render::RenderContext* m_renderContext;
};

}
