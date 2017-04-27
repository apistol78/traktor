/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_TriggerGrainData_H
#define traktor_sound_TriggerGrainData_H

#include "Sound/Types.h"
#include "Sound/Resound/IGrainData.h"

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

class T_DLLCLASS TriggerGrainData : public IGrainData
{
	T_RTTI_CLASS;

public:
	TriggerGrainData();

	virtual Ref< IGrain > createInstance(IGrainFactory* grainFactory) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const std::wstring& getId() const { return m_id; }

	float getPosition() const { return m_position; }

	float getRate() const { return m_rate; }

	IGrainData* getGrain() const { return m_grain; }

private:
	std::wstring m_id;
	float m_position;
	float m_rate;
	Ref< IGrainData > m_grain;
};

	}
}

#endif	// traktor_sound_TriggerGrainData_H
