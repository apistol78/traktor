/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

class ISoundListener;
class ISoundPlayer;

}

namespace traktor::spray
{

/*! Sound listener component instance.
 * \ingroup Spray
 */
class T_DLLCLASS ListenerComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit ListenerComponent(sound::ISoundPlayer* soundPlayer);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void setEnable(bool enable);

	bool isEnable() const;

private:
	Ref< sound::ISoundPlayer > m_soundPlayer;
	Ref< sound::ISoundListener > m_soundListener;
	Transform m_transform;
};

}
