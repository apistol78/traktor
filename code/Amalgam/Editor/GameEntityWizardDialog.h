#ifndef traktor_amalgam_GameEntityWizardDialog_H
#define traktor_amalgam_GameEntityWizardDialog_H

#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace db
	{

class Group;

	}

	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class CheckBox;
class DropDown;
class Edit;

	}

	namespace amalgam
	{

class GameEntityWizardDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	GameEntityWizardDialog(editor::IEditor* editor, db::Group* group);

	bool create(ui::Widget* parent);

private:
	editor::IEditor* m_editor;
	Ref< db::Group > m_group;
	Ref< ui::Edit > m_editName;
	Ref< ui::Edit > m_editVisualMesh;
	Ref< ui::Edit > m_editCollisionMesh;
	Ref< ui::DropDown > m_dropPhysicsType;
	Ref< ui::Edit > m_editCollisionGroup;
	Ref< ui::Edit > m_editCollisionMask;
	Ref< ui::Edit > m_editMaterial;
	Ref< ui::Edit > m_editFriction;
	Ref< ui::CheckBox > m_checkBoxCreateGroup;
	Ref< ui::CheckBox > m_checkBoxCreateScript;
	bool m_nameEdited;

	void eventNameChange(ui::ContentChangeEvent* event);

	void eventBrowseVisualMeshClick(ui::ButtonClickEvent* event);

	void eventBrowseCollisionMeshClick(ui::ButtonClickEvent* event);

	void eventDialogClick(ui::ButtonClickEvent* event);
};

	}
}

#endif	// traktor_amalgam_GameEntityWizardDialog_H
