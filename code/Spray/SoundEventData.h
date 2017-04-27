/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_SoundEventData_H
#define traktor_spray_SoundEventData_H

#include "Resource/Id.h"
#include "World/IEntityEventData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class Sound;

	}

	namespace spray
	{

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS SoundEventData : public world::IEntityEventData
{
	T_RTTI_CLASS;

public:
	SoundEventData();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class EffectEntityFactory;
	friend class EffectEntityPipeline;

	resource::Id< sound::Sound > m_sound;
	bool m_positional;
	bool m_follow;
	bool m_autoStopFar;
};

	}
}

#endif	// traktor_spray_SoundEventData_H
