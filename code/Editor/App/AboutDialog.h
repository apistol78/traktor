#ifndef traktor_editor_AboutDialog_H
#define traktor_editor_AboutDialog_H

#include "Ui/Dialog.h"

namespace traktor
{
	namespace editor
	{

class AboutDialog : public ui::Dialog
{
	T_RTTI_CLASS(AboutDialog)

public:
	bool create(ui::Widget* parent);
};

	}
}

#endif	// traktor_editor_AboutDialog_H
