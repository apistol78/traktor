/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_ISoundBuffer_H
#define traktor_sound_ISoundBuffer_H

#include "Core/Object.h"
#include "Sound/Types.h"

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

class ISoundMixer;

/*! \brief Sound buffer cursor interface.
 * \ingroup Sound
 */
class T_DLLCLASS ISoundBufferCursor : public IRefCount
{
public:
	virtual void setParameter(handle_t id, float parameter) = 0;

	virtual void disableRepeat() = 0;

	virtual void reset() = 0;
};

/*! \brief Sound buffer base class.
 * \ingroup Sound
 */
class T_DLLCLASS ISoundBuffer : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< ISoundBufferCursor > createCursor() const = 0;

	virtual bool getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const = 0;
};

	}
}

#endif	// traktor_sound_ISoundBuffer_H
