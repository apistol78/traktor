/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_SequenceGrain_H
#define traktor_sound_SequenceGrain_H

#include "Core/RefArray.h"
#include "Sound/Resound/IGrain.h"

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

/*! \brief
 * \ingroup Sound
 */
class T_DLLCLASS SequenceGrain : public IGrain
{
	T_RTTI_CLASS;

public:
	SequenceGrain(const RefArray< IGrain >& grains);

	virtual Ref< ISoundBufferCursor > createCursor() const T_OVERRIDE T_FINAL;

	virtual void updateCursor(ISoundBufferCursor* cursor) const T_OVERRIDE T_FINAL;

	virtual const IGrain* getCurrentGrain(const ISoundBufferCursor* cursor) const T_OVERRIDE T_FINAL;

	virtual void getActiveGrains(const ISoundBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const T_OVERRIDE T_FINAL;

	virtual bool getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const T_OVERRIDE T_FINAL;

private:
	RefArray< IGrain > m_grains;
};

	}
}

#endif	// traktor_sound_SequenceGrain_H
