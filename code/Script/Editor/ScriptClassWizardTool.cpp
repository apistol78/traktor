#include "Core/Io/StringOutputStream.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "I18N/Text.h"
#include "Script/Editor/Script.h"
#include "Script/Editor/ScriptClassWizardTool.h"
#include "Ui/CharacterSetEditValidator.h"
#include "Ui/Custom/InputDialog.h"

namespace traktor
{
	namespace script
	{

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
			0
		},
		{
			i18n::Text(L"SCRIPT_CLASS_WIZARDTOOL_INHERIT_CLASS_NAME"),
			L"",
			&classNameValidator,
			0
		}
	};

	inputDialog.create(
		parent,
		i18n::Text(L"SCRIPT_CLASS_WIZARDTOOL_DIALOG_TITLE"),
		i18n::Text(L"SCRIPT_CLASS_WIZARDTOOL_DIALOG_MESSAGE"),
		fields,
		sizeof_array(fields)
	);

	if (inputDialog.showModal() != ui::DrOk)
	{
		inputDialog.destroy();
		return false;
	}

	inputDialog.destroy();

	// Create script class.
	std::wstring className = fields[0].value;
	std::wstring inheritClassName = fields[1].value;

	StringOutputStream ss;
	ss << L"-- " << className << Endl;
	ss << Endl;
	if (inheritClassName.empty())
		ss << className << L" = " << className << L" or {}" << Endl;
	else
		ss << className << L" = " << className << L" or inherit(" << inheritClassName << L")" << Endl;
	ss << className << L"_meta = " << className << L"_meta or { __index = " << className << L" }" << Endl;
	ss << Endl;
	ss << L"function " << className << L".new(o)" << Endl;
	ss << L"\to = o or {}" << Endl;
	ss << L"\tsetmetatable(o, " << className << L"_meta)" << Endl;
	ss << Endl;
	ss << L"\treturn o" << Endl;
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
