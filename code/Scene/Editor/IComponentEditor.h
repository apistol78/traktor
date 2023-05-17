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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class PrimitiveRenderer;

}

namespace traktor::scene
{

class EntityAdapter;

/*!
 * \ingroup Scene
 */
class T_DLLCLASS IComponentEditor : public Object
{
	T_RTTI_CLASS;

public:
	virtual void transformModified(EntityAdapter* entityAdapter, EntityAdapter* modifiedEntityAdapter) = 0;

	/*! Draw guide for component.
	 *
	 * A guide is the wire overlay
	 * in the 3d editor view.
	 *
	 * \param primitiveRenderer Primitive wire renderer.
	 */
	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const = 0;
};

}
