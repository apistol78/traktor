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
#include "Core/RefArray.h"
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

class IGrainData;

class T_DLLCLASS BankAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addGrain(IGrainData* grain);

	void removeGrain(IGrainData* grain);

	const RefArray< IGrainData >& getGrains() const;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class BankPipeline;

	Guid m_category;
	float m_presence = 0.0f;
	float m_presenceRate = 0.25f;
	RefArray< IGrainData > m_grains;
};

}
