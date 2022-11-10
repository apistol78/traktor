/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Guid;

	namespace world
	{

class Entity;

	}

	namespace scene
	{

class ISceneController;

/*! Scene controller data.
 * \ingroup Scene
 *
 * Scene controller data is a persistent
 * means of storing specific scene controller
 * data on disc.
 */
class T_DLLCLASS ISceneControllerData : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Create controller instance.
	 *
	 * \param entityProducts Entites created from entity data.
	 * \param editor True if scene is created in editor.
	 * \return Controller instance.
	 */
	virtual Ref< ISceneController > createController(const SmallMap< Guid, Ref< world::Entity > >& entityProducts, bool editor) const = 0;
};

	}
}

