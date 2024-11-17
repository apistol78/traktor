/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/ConfigDialog.h"

namespace traktor::db
{

class Group;

}

namespace traktor::editor
{

class IEditor;

}

namespace traktor::ui
{

class CheckBox;
class DropDown;
class Edit;

}

namespace traktor::runtime
{

class GameEntityWizardDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	explicit GameEntityWizardDialog(editor::IEditor* editor, db::Group* group);

	bool create(ui::Widget* parent);

private:
	editor::IEditor* m_editor;
	Ref< db::Group > m_group;
	Ref< ui::Edit > m_editName;
	Ref< ui::Edit > m_editVisualMesh;
	Ref< ui::Edit > m_editSkeletonMesh;
	Ref< ui::Edit > m_editAnimationMesh;
	Ref< ui::Edit > m_editCollisionMesh;
	Ref< ui::DropDown > m_dropEntityType;
	Ref< ui::DropDown > m_dropCollisionGroup;
	Ref< ui::DropDown > m_dropCollisionMask;
	Ref< ui::Edit > m_editMaterial;
	Ref< ui::Edit > m_editFriction;
	Ref< ui::Edit > m_editScale;
	Ref< ui::CheckBox > m_checkBoxCreateScript;
	bool m_nameEdited;

	void eventNameChange(ui::ContentChangeEvent* event);

	void eventBrowseVisualMeshClick(ui::ButtonClickEvent* event);

	void eventCopyVisualMeshClick(ui::ButtonClickEvent* event);

	void eventBrowseSkeletonMeshClick(ui::ButtonClickEvent* event);

	void eventCopySkeletonMeshClick(ui::ButtonClickEvent* event);

	void eventBrowseAnimationMeshClick(ui::ButtonClickEvent* event);

	void eventCopyAnimationMeshClick(ui::ButtonClickEvent* event);

	void eventBrowseCollisionMeshClick(ui::ButtonClickEvent* event);

	void eventCopyCollisionMeshClick(ui::ButtonClickEvent* event);

	void eventDialogClick(ui::ButtonClickEvent* event);
};

}
