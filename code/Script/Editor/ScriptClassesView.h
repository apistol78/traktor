#ifndef traktor_script_ScriptClassesView_H
#define traktor_script_ScriptClassesView_H

#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class TreeView;

		}
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

	virtual void destroy() T_OVERRIDE T_FINAL;

private:
	Ref< ui::custom::TreeView > m_treeClasses;
};

	}
}

#endif	// traktor_script_ScriptClassesView_H
