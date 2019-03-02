#pragma once

#include <string>
#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class T_DLLCLASS SearchEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	SearchEvent(
		ui::EventSubject* sender,
		bool preview
	);

	bool preview() const { return m_preview; }

private:
	std::wstring m_search;
	bool m_preview;
};

	}
}

