#pragma once

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

/*! \brief ToolForm interface.
 * \ingroup UI
 */
class IToolForm : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) = 0;

	virtual int showModal() = 0;

	virtual void endModal(int result) = 0;
};

	}
}

