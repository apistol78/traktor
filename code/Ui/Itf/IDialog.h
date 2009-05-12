#ifndef traktor_ui_IDialog_H
#define traktor_ui_IDialog_H

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
	namespace drawing
	{

class Image;

	}

	namespace ui
	{

/*! \brief Dialog interface.
 * \ingroup UI
 */
class T_DLLCLASS IDialog : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) = 0;

	virtual void setIcon(drawing::Image* icon) = 0;
	
	virtual int showModal() = 0;

	virtual void endModal(int result) = 0;

	virtual void setMinSize(const Size& minSize) = 0;
};

	}
}

#endif	// traktor_ui_IDialog_H
