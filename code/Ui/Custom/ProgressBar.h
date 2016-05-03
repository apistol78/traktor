#ifndef traktor_ui_custom_ProgressBar_H
#define traktor_ui_custom_ProgressBar_H

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

/*! \brief Progress bar.
 * \ingroup UIC
 */
class T_DLLCLASS ProgressBar : public Widget
{
	T_RTTI_CLASS;

public:
	ProgressBar();

	bool create(Widget* parent, int32_t style = WsNone, int32_t minProgress = 0, int32_t maxProgress = 100);

	void setRange(int32_t minProgress, int32_t maxProgress);

	int32_t getMinRange() const;

	int32_t getMaxRange() const;

	void setProgress(int progress);

	int32_t getProgress() const;

	virtual Size getPreferedSize() const T_OVERRIDE;

private:
	int32_t m_minProgress;
	int32_t m_maxProgress;
	int32_t m_progress;
	int32_t m_loop;

	void eventPaint(PaintEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ProgressBar_H
