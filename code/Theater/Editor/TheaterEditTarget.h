/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Transform.h"

#include <string>

namespace traktor::scene
{

class EntityAdapter;

}

namespace traktor::theater
{

class ActData;
class Performance;

/*! Theater component being edited.
 *
 * Abstract the differences between animating the entire world and
 * animating child entities of a single entity, so the editor can
 * present and edit both alike.
 */
class TheaterEditTarget : public Object
{
	T_RTTI_CLASS;

public:
	/*! Name of target as presented to the user. */
	virtual std::wstring getName() const = 0;

	/*! Identifier of target; used to keep selection across rebuilds of the scene. */
	virtual Guid getId() const = 0;

	/*! Acts of target. */
	virtual RefArray< ActData >& getActs() = 0;

	/*! Transform of which all tracks of this target are relative. */
	virtual Transform getBaseTransform() const = 0;

	/*! Find entity, animated by this target, from identifier. */
	virtual scene::EntityAdapter* findEntityAdapter(const Guid& id) const = 0;

	/*! Check if entity can be animated by this target. */
	virtual bool canCapture(const scene::EntityAdapter* entityAdapter) const = 0;

	/*! Get performance of the instantiated component; null if not instantiated. */
	virtual Performance* getPerformance() const = 0;

	/*! Re-instantiate component; called after acts have been modified. */
	virtual void rebuild() = 0;
};

}
