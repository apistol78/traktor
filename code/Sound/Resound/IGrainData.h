/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_IGrainData_H
#define traktor_sound_IGrainData_H

#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

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

class IGrain;
class IGrainFactory;

class T_DLLCLASS IGrainData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< IGrain > createInstance(IGrainFactory* grainFactory) const = 0;
};

	}
}

#endif	// traktor_sound_IGrainData_H
