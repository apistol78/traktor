/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
class Instance;

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
	T_RTTI_CLASS;

public:
	enum WizardFlags
	{
		WfGroup = 1,
		WfInstance = 2
	};

	virtual std::wstring getDescription() const = 0;

	virtual uint32_t getFlags() const = 0;

	virtual bool launch(ui::Widget* parent, IEditor* editor, db::Group* group, db::Instance* instance) = 0;
};

	}
}

#endif	// traktor_editor_IWizardTool_H
