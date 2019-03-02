#pragma once

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

/*! \brief UserWidget interface.
 * \ingroup UI
 */
class IUserWidget : public IWidget
{
public:
	virtual bool create(IWidget* parent, int style) = 0;
};

	}
}

