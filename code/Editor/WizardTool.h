#ifndef traktor_editor_WizardTool_H
#define traktor_editor_WizardTool_H

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

class Editor;

class T_DLLCLASS WizardTool : public Object
{
	T_RTTI_CLASS(WizardTool)

public:
	virtual std::wstring getDescription() const = 0;

	virtual bool launch(ui::Widget* parent, Editor* editor, db::Group* group) = 0;
};

	}
}

#endif	// traktor_editor_WizardTool_H
