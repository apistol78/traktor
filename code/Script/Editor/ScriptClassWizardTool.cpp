/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/StringOutputStream.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Script/Editor/Script.h"
#include "Script/Editor/ScriptClassWizardTool.h"
#include "Ui/CharacterSetEditValidator.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Custom/InputDialog.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

class ClassValueEnumerator : public RefCountImpl< ui::custom::InputDialog::IValueEnumerator >
{
public:
	ClassValueEnumerator(const RefArray< db::Instance >& instances)
	:	m_instances(instances)
	{
	}

	virtual bool getValue(size_t index, std::wstring& outKey, std::wstring& outValue) const T_OVERRIDE T_FINAL
	{
		if (index < m_instances.size())
		{
			if (m_instances[index])
				outKey = outValue = m_instances[index]->getPath();
			else
			{
				outKey = i18n::Text(L"SCRIPT_CLASS_NO_CLASS");
				outValue = L"";
			}
			return true;
		}
		else
			return false;
	}

private:
	const RefArray< db::Instance >& m_instances;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptClassWizardTool", 0, ScriptClassWizardTool, editor::IWizardTool)

std::wstring ScriptClassWizardTool::getDescription() const
{
	return i18n::Text(L"SCRIPT_CLASS_WIZARDTOOL_DESCRIPTION");
}

uint32_t ScriptClassWizardTool::getFlags() const
{
	return WfGroup;
}

bool ScriptClassWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	RefArray< db::Instance > scriptInstances;
	scriptInstances.push_back(nullptr);

	// Find all script instances; name of script is same as class name.
	Ref< db::Database > database = editor->getSourceDatabase();
	db::recursiveFindChildInstances(database->getRootGroup(), db::FindInstanceByType(type_of< Script >()), scriptInstances);

	ui::CharacterSetEditValidator classNameValidator;
	classNameValidator.addRange(L'a', L'z');
	classNameValidator.addRange(L'A', L'Z');
	classNameValidator.addRange(L'0', L'9');
	classNameValidator.add(L'_');
	classNameValidator.add(L'.');

	ui::custom::InputDialog inputDialog;
	ui::custom::InputDialog::Field fields[] =
	{
		{
			i18n::Text(L"SCRIPT_CLASS_WIZARDTOOL_CLASS_NAME"),
			L"",
			&classNameValidator,
			nullptr
		},
		{
			i18n::Text(L"SCRIPT_CLASS_WIZARDTOOL_INHERIT_CLASS_NAME"),
			L"",
			&classNameValidator,
			new ClassValueEnumerator(scriptInstances)
		}
	};

	inputDialog.create(
		parent,
		i18n::Text(L"SCRIPT_CLASS_WIZARDTOOL_DIALOG_TITLE"),
		i18n::Text(L"SCRIPT_CLASS_WIZARDTOOL_DIALOG_MESSAGE"),
		fields,
		sizeof_array(fields)
	);

	inputDialog.setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	if (inputDialog.showModal() != ui::DrOk)
	{
		inputDialog.destroy();
		return false;
	}

	inputDialog.destroy();

	// Create script class.
	std::wstring className = fields[0].value;
	Ref< db::Instance > inheritClassInstance;

	if (!fields[1].value.empty())
	{
		inheritClassInstance = database->getInstance(fields[1].value);
		if (!inheritClassInstance)
			return false;
	}

	StringOutputStream ss;
	
	if (inheritClassInstance != nullptr)
		ss << L"#using \\" << inheritClassInstance->getGuid().format() << Endl;

	ss << Endl;
	if (inheritClassInstance != nullptr)
		ss << className << L" = " << className << L" or class(\"" << className << L"\", " << inheritClassInstance->getName() << L")" << Endl;
	else
		ss << className << L" = " << className << L" or class(\"" << className << L"\")" << Endl;
	ss << Endl;
	ss << L"function " << className << L":new()" << Endl;
	if (inheritClassInstance != nullptr)
		ss << L"\t" << inheritClassInstance->getName() << L".new(self)" << Endl;
	ss << L"end" << Endl;

	Ref< Script > s = new Script(ss.str());

	// Create database instance.
	Ref< db::Instance > scriptInstance = group->createInstance(
		className,
		db::CifReplaceExisting | db::CifKeepExistingGuid
	);
	if (!scriptInstance)
		return false;

	scriptInstance->setObject(s);

	if (!scriptInstance->commit())
		return false;

	return true;
}

	}
}
