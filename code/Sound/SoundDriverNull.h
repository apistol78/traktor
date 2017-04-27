/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_SoundDriverNull_H
#define traktor_sound_SoundDriverNull_H

#include "Sound/ISoundDriver.h"

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

/*! \brief Null device sound driver.
 * \ingroup Sound
 */
class T_DLLCLASS SoundDriverNull : public ISoundDriver
{
	T_RTTI_CLASS;

public:
	virtual bool create(const SystemApplication& sysapp, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void wait() T_OVERRIDE T_FINAL;

	virtual void submit(const SoundBlock& soundBlock) T_OVERRIDE T_FINAL;

private:
	SoundDriverCreateDesc m_desc;
};

	}
}

#endif	// traktor_sound_SoundDriverNull_H
