/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/Editor/IStreamEncoder.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class OggStreamEncoderImpl;

/*! OGG stream encoder.
 * \ingroup Sound
 */
class T_DLLCLASS OggStreamEncoder : public IStreamEncoder
{
	T_RTTI_CLASS;

public:
	virtual bool create(IStream* stream) override final;

	virtual void destroy() override final;

	virtual bool putBlock(SoundBlock& block) override final;

private:
	Ref< OggStreamEncoderImpl > m_impl;
};

	}
}

