#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/Split.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "I18N/Dictionary.h"
#include "I18N/Text.h"
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

bool ImportDictionaryWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	std::wstring line;

	ui::FileDialog fileDialog;
	if (!fileDialog.create(parent, Text(L"IMPORT_DICTIONARY_WIZARDTOOL_FILE_TITLE"), L"All files;*.*"))
		return 0;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DrOk)
	{
		fileDialog.destroy();
		return true;
	}
	fileDialog.destroy();

	// Assume CSV format.
	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!file)
		return false;

	Ref< Dictionary > dictionary = new Dictionary();

	StringReader sr(file, new Utf8Encoding());
	while (sr.readLine(line) >= 0)
	{
		std::vector< std::wstring > columns;
		Split< std::wstring >::any(line, L",", columns, true);
		if (columns.size() >= 2 && !columns[0].empty())
		{
			dictionary->set(
				columns[0],
				columns[1]
			);
		}
	}

	file->close();
	file = 0;

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
