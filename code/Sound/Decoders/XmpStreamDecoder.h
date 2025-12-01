/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/IStreamDecoder.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

/*! \ingroup Sound */
//@{

/*!
 */
class T_DLLCLASS XmpStreamDecoder : public IStreamDecoder
{
	T_RTTI_CLASS;

public:
	XmpStreamDecoder();

	virtual bool create(IStream* stream) override final;

	virtual void destroy() override final;

	virtual double getDuration() const override final;

	virtual bool getBlock(AudioBlock& outBlock) override final;

	virtual void rewind() override final;

private:
	float* m_samplesBuffer[SbcMaxChannelCount];
	void* m_ctx = nullptr;
	double channel_offsets[32];
};

//@}

}
