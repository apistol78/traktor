#pragma once

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

class ISystemBitmap;

/*! \brief Dialog interface.
 * \ingroup UI
 */
class IDialog : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) = 0;

	virtual void setIcon(ISystemBitmap* icon) = 0;

	virtual int showModal() = 0;

	virtual void endModal(int result) = 0;

	virtual void setMinSize(const Size& minSize) = 0;
};

	}
}

