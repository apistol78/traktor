/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class T_DLLCLASS SoundCategory : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	const Guid& getParent() const { return m_parent; }

	const std::wstring& getConfigurationId() const { return m_configurationId; }

	float getGain() const { return m_gain; }

	float getPresence() const { return m_presence; }

	float getPresenceRate() const { return m_presenceRate; }

	float getRange() const { return m_range; }

private:
	Guid m_parent;
	std::wstring m_configurationId;
	float m_gain = 0.0f;
	float m_presence = 0.0f;
	float m_presenceRate = 0.25f;
	float m_range = 0.0f;
};

}
