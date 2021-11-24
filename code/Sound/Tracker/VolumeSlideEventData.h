#pragma once

#include "Sound/Tracker/IEventData.h"

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

class T_DLLCLASS VolumeSlideEventData : public IEventData
{
	T_RTTI_CLASS;

public:
	VolumeSlideEventData();

	VolumeSlideEventData(float amount);

	virtual Ref< IEvent > createInstance() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_amount;
};

	}
}
