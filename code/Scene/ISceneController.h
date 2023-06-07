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
#if defined(T_SCENE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::scene
{

class Scene;

/*! Scene controller interface.
 * \ingroup Scene
 *
 * Scene controllers are data-driven controllers
 * which can be associated with a scene.
 * Controllers are able to access all entities defined
 * in the scene which makes it possible to create for
 * instance cut-scene controllers etc. without involving
 * too much game logic.
 */
class T_DLLCLASS ISceneController : public Object
{
	T_RTTI_CLASS;

public:
	virtual void update(Scene* scene, double time, double deltaTime) = 0;
};

}
