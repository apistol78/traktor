#pragma once

#include <string>
#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class T_DLLCLASS MeasurementEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	MeasurementEvent(ui::EventSubject* sender, int32_t pass, int32_t level, const std::wstring& name, double start, double duration);

    int32_t getPass() const { return m_pass; }

    int32_t getLevel() const { return m_level; }

    const std::wstring& getName() const { return m_name; }

    double getStart() const { return m_start; }

    double getDuration() const { return m_duration; }

private:
    int32_t m_pass;
    int32_t m_level;
    std::wstring m_name;
    double m_start;
    double m_duration;
};

	}
}

