/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::sound
{

class ISoundPlayer;
class Sound;

}

namespace traktor::spray
{

class SoundComponent;

/*! Sound component data.
 * \ingroup Spray
 */
class T_DLLCLASS SoundComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	SoundComponentData() = default;

	explicit SoundComponentData(const resource::Id< sound::Sound >& sound);

	Ref< SoundComponent > createComponent(resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer) const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< sound::Sound >& getSound() const { return m_sound; }

private:
	resource::Id< sound::Sound > m_sound;
};

}
