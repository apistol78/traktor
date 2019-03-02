#pragma once

#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{

class TreeView;

	}

	namespace script
	{

/*! \brief Runtime classes view.
 * \ingroup Script
 */
class ScriptClassesView : public ui::Container
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	virtual void destroy() override final;

private:
	Ref< ui::TreeView > m_treeClasses;
};

	}
}

