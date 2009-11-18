#ifndef traktor_ui_custom_ProfileControl_H
#define traktor_ui_custom_ProfileControl_H

#include <list>
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Timer;

	namespace ui
	{

class Event;

		namespace custom
		{

/*! \brief Memory profiler control.
 * \ingroup UIC
 */
class T_DLLCLASS ProfileControl : public Widget
{
	T_RTTI_CLASS;

public:
	ProfileControl();

	bool create(Widget* parent, int time, int minSample, int maxSample, int style);

	virtual Size getPreferedSize() const;

private:
	struct Sample
	{
		double at;
		uint32_t objectCount;
		uint32_t referenceCount;
	};

	Ref< Timer > m_timer;
	std::list< Sample > m_samples;
	uint32_t m_peekObjectReference;

	void eventPaint(Event* event);

	void eventTimer(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ProfileControl_H
