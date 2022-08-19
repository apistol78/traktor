#pragma once

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

class ISystemBitmap;

/*! ToolForm interface.
 * \ingroup UI
 */
class IToolForm : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) = 0;

	virtual void setIcon(ISystemBitmap* icon) = 0;

	virtual DialogResult showModal() = 0;

	virtual void endModal(DialogResult result) = 0;
};

	}
}

