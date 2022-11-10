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

namespace traktor
{
	namespace world
	{

class Entity;

	}

	namespace scene
	{

class EntityAdapter;
class SceneEditorContext;

/*! Entity adapter lookup from entity accelerator.
 *
 * This class is used by the special entity renderer, EntityRendererAdapter,
 * in order to quickly lookup entity adapter from entity.
 *
 * This is performed by knowing ahead of time which traversal path the
 * renderer is performing thus only accessing children adapters
 * instead of full search.
 */
class EntityRendererCache : public Object
{
	T_RTTI_CLASS;

public:
	EntityRendererCache(SceneEditorContext* context);

	EntityAdapter* begin(const Object* renderable);

	void end();

private:
	SceneEditorContext* m_context;
};

	}
}

