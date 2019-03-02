#pragma once

#include "Ui/Form.h"

namespace traktor
{
	namespace editor
	{

/*! \brief
 * \ingroup Editor
 */
class EditorPageForm : public ui::Form
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	virtual void destroy() override;
};

	}
}

