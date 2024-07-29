/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class SoundPlayer;

}

namespace traktor::spray
{

class ListenerComponent;

/*! Sound listener component data.
 * \ingroup Spray
 */
class T_DLLCLASS ListenerComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< ListenerComponent > createComponent(sound::SoundPlayer* soundPlayer) const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;
};

}
