/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Group.h"
#include "Database/Instance.h"
#include "I18N/Dictionary.h"
#include "I18N/Text.h"
#include "I18N/Editor/IDictionaryFormat.h"
#include "I18N/Editor/ImportDictionaryWizardTool.h"
#include "Ui/FileDialog.h"

namespace traktor
{
	namespace i18n
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.i18n.ImportDictionaryWizardTool", 0, ImportDictionaryWizardTool, editor::IWizardTool)

std::wstring ImportDictionaryWizardTool::getDescription() const
{
	return Text(L"IMPORT_DICTIONARY_WIZARDTOOL_DESCRIPTION");
}

uint32_t ImportDictionaryWizardTool::getFlags() const
{
	return WfGroup;
}

const TypeInfoSet ImportDictionaryWizardTool::getSupportedTypes() const
{
	return TypeInfoSet();
}

bool ImportDictionaryWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	std::wstring line;

	ui::FileDialog fileDialog;
	if (!fileDialog.create(parent, type_name(this), Text(L"IMPORT_DICTIONARY_WIZARDTOOL_FILE_TITLE"), L"All files;*.*"))
		return false;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DialogResult::Ok)
	{
		fileDialog.destroy();
		return false;
	}
	fileDialog.destroy();

	Ref< Dictionary > dictionary = IDictionaryFormat::readAny(fileName, 0, 1);
	if (!dictionary)
		return false;

	// Create dictionary instance.
	Ref< db::Instance > dictionaryInstance = group->createInstance(
		fileName.getFileNameNoExtension(),
		db::CifReplaceExisting | db::CifKeepExistingGuid
	);
	if (!dictionaryInstance)
		return false;

	dictionaryInstance->setObject(dictionary);

	if (!dictionaryInstance->commit())
		return false;

	return true;
}

	}
}
