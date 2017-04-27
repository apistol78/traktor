/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_ISoundHandle_H
#define traktor_sound_ISoundHandle_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Vector4;

	namespace sound
	{

class T_DLLCLASS ISoundHandle : public Object
{
	T_RTTI_CLASS;

public:
	virtual void stop() = 0;

	virtual void fadeOff() = 0;

	virtual bool isPlaying() = 0;

	virtual void setVolume(float volume) = 0;

	virtual void setPitch(float pitch) = 0;

	virtual void setPosition(const Vector4& position) = 0;

	virtual void setParameter(int32_t id, float parameter) = 0;
};

	}
}

#endif	// traktor_sound_ISoundHandle_H
