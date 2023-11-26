/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Sound/Types.h"
#include "Sound/Resound/IGrainData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class T_DLLCLASS EnvelopeGrainData : public IGrainData
{
	T_RTTI_CLASS;

public:
	struct GrainData
	{
		Ref< IGrainData > grain;
		float in = 0.0f;
		float out = 0.0f;
		float easeIn = 0.2f;
		float easeOut = 0.2f;

		void serialize(ISerializer& s);
	};

	EnvelopeGrainData();

	void addGrain(IGrainData* grain, float in, float out, float easeIn, float easeOut);

	void removeGrain(IGrainData* grain);

	void setLevels(const float levels[3]);

	void setMid(float mid);

	void setResponse(float response);

	virtual Ref< IGrain > createInstance(IGrainFactory* grainFactory) const override final;

	virtual void serialize(ISerializer& s) override final;

	const std::wstring& getId() const { return m_id; }

	const AlignedVector< GrainData >& getGrains() const { return m_grains; }

	const float* getLevels() const { return m_levels; }

	float getMid() const { return m_mid; }

	float getResponse() const { return m_response; }

private:
	std::wstring m_id;
	AlignedVector< GrainData > m_grains;
	float m_levels[3];
	float m_mid;
	float m_response;
};

}
