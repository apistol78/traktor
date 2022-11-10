/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/IFilter.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class SurroundEnvironment;

/*! Surround filter.
 * \ingroup Sound
 */
class T_DLLCLASS SurroundFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	SurroundFilter(SurroundEnvironment* environment, const Vector4& speakerPosition, float maxDistance);

	void setSpeakerPosition(const Vector4& speakerPosition);

	void setMaxDistance(float maxDistance);

	virtual Ref< IFilterInstance > createInstance() const override final;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< SurroundEnvironment > m_environment;
	Vector4 m_speakerPosition;
	Scalar m_maxDistance;

	void applyStereo(IFilterInstance* instance, SoundBlock& outBlock) const;

	void applyFull(IFilterInstance* instance, SoundBlock& outBlock) const;
};

	}
}

