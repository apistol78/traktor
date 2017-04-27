/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_OggStreamEncoder_H
#define traktor_sound_OggStreamEncoder_H

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

/*! \brief OGG stream encoder.
 * \ingroup Sound
 */
class T_DLLCLASS OggStreamEncoder : public IStreamEncoder
{
	T_RTTI_CLASS;

public:
	virtual bool create(IStream* stream) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool putBlock(SoundBlock& block) T_OVERRIDE T_FINAL;

private:
	Ref< OggStreamEncoderImpl > m_impl;
};

	}
}

#endif	// traktor_sound_OggStreamEncoder_H
