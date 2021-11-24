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

class T_DLLCLASS GotoEventData : public IEventData
{
	T_RTTI_CLASS;

public:
	GotoEventData();

	GotoEventData(int32_t pattern, int32_t row);

	virtual Ref< IEvent > createInstance() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_pattern;
	int32_t m_row;
};

	}
}
