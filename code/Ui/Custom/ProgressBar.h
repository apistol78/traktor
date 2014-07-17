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

class Bitmap;

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

	bool create(Widget* parent, int style = WsNone, int minProgress = 0, int maxProgress = 100);

	void setRange(int minProgress, int maxProgress);

	int getMinRange() const;

	int getMaxRange() const;

	void setProgress(int progress);

	int getProgress() const;

	virtual Size getPreferedSize() const;

private:
	Ref< Bitmap > m_imageProgressBar;
	int m_minProgress;
	int m_maxProgress;
	int m_progress;
	int m_loop;

	void eventPaint(PaintEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ProgressBar_H
