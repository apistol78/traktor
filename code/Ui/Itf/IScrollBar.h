#ifndef traktor_ui_IScrollBar_H
#define traktor_ui_IScrollBar_H

#include "Ui/Itf/IWidget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief ScrollBar interface.
 * \ingroup UI
 */
class T_DLLCLASS IScrollBar : public IWidget
{
public:
	virtual bool create(IWidget* parent, int style) = 0;

	virtual void setRange(int range) = 0;

	virtual int getRange() const = 0;

	virtual void setPage(int page) = 0;

	virtual int getPage() const = 0;

	virtual void setPosition(int position) = 0;

	virtual int getPosition() const = 0;
};

	}
}

#endif	// traktor_ui_IScrollBar_H
