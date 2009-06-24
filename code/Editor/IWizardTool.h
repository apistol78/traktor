#ifndef traktor_editor_IWizardTool_H
#define traktor_editor_IWizardTool_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Widget;

	}

	namespace db
	{

class Group;

	}

	namespace editor
	{

class IEditor;

/*! \brief Wizard tool base class.
 * \ingroup Editor
 *
 * Wizards are launched by context menu
 * from database view.
 */
class T_DLLCLASS IWizardTool : public Object
{
	T_RTTI_CLASS(IWizardTool)

public:
	virtual std::wstring getDescription() const = 0;

	virtual bool launch(ui::Widget* parent, IEditor* editor, db::Group* group) = 0;
};

	}
}

#endif	// traktor_editor_IWizardTool_H
