/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class ISoundHandle;
class ISoundPlayer;
class Sound;

}

namespace traktor::spray
{

/*! Sound component instance.
 * \ingroup Spray
 */
class T_DLLCLASS SoundComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit SoundComponent(sound::ISoundPlayer* soundPlayer, const resource::Proxy< sound::Sound >& sound);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void play();

	void stop();

	void setVolume(float volume);

	void setPitch(float pitch);

	void setParameter(const std::wstring& id, float parameter);

private:
	Ref< sound::ISoundPlayer > m_soundPlayer;
	resource::Proxy< sound::Sound > m_sound;
	Ref< sound::ISoundHandle > m_handle;
	Transform m_transform;
};

}
