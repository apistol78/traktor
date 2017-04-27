/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_IStreamEncoder_H
#define traktor_sound_IStreamEncoder_H

#include "Core/Object.h"
#include "Sound/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace sound
	{

/*! \brief Sound stream encoder.
 * \ingroup Sound
 */
class T_DLLCLASS IStreamEncoder : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(IStream* stream) = 0;

	virtual void destroy() = 0;

	virtual bool putBlock(SoundBlock& block) = 0;
};

	}
}

#endif	// traktor_sound_IStreamEncoder_H
