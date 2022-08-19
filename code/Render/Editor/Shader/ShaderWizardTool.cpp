#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderWizardTool.h"
#include "Ui/InputDialog.h"
#include "Ui/StyleBitmap.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class ShaderValueEnumerator : public RefCountImpl< ui::InputDialog::IValueEnumerator >
{
public:
	ShaderValueEnumerator(const RefArray< db::Instance >& instances)
	:	m_instances(instances)
	{
	}

	virtual bool getValue(size_t index, std::wstring& outKey, std::wstring& outValue) const override final
	{
		if (index < m_instances.size())
		{
			if (m_instances[index])
			{
				outKey = m_instances[index]->getName();
				outValue = m_instances[index]->getPath();
			}
			else
			{
				outKey = i18n::Text(L"RENDER_SHADER_WIZARDTOOL_EMPTY");
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderWizardTool", 0, ShaderWizardTool, editor::IWizardTool)

std::wstring ShaderWizardTool::getDescription() const
{
	return i18n::Text(L"RENDER_SHADER_WIZARDTOOL_DESCRIPTION");
}

uint32_t ShaderWizardTool::getFlags() const
{
	return WfGroup;
}

bool ShaderWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	Ref< db::Database > database = editor->getSourceDatabase();

	// Get group which contain all template shaders.
	Ref< db::Group > shaderGroup = database->getGroup(L"System/Shaders/Wizard");
	if (!shaderGroup)
		return false;

	// Collect all shader template instances.
	RefArray< db::Instance > shaderInstances;
	db::recursiveFindChildInstances(shaderGroup, db::FindInstanceByType(type_of< ShaderGraph >()), shaderInstances);
	if (shaderInstances.empty())
		return false;

	ui::InputDialog inputDialog;
	ui::InputDialog::Field fields[] =
	{
		{
			i18n::Text(L"RENDER_SHADER_WIZARDTOOL_SHADER_TEMPLATE"),
			L"",
			nullptr,
			new ShaderValueEnumerator(shaderInstances)
		},
		{
			i18n::Text(L"RENDER_SHADER_WIZARDTOOL_NAME"),
			L"",
			nullptr,
			nullptr
		},
	};

	inputDialog.create(
		parent,
		i18n::Text(L"RENDER_SHADER_WIZARDTOOL_DIALOG_TITLE"),
		i18n::Text(L"RENDER_SHADER_WIZARDTOOL_DIALOG_MESSAGE"),
		fields,
		sizeof_array(fields)
	);

	inputDialog.setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	if (inputDialog.showModal() != ui::DialogResult::Ok)
	{
		inputDialog.destroy();
		return false;
	}

	inputDialog.destroy();

	std::wstring shaderName = fields[1].value;

	Ref< db::Instance > shaderInstance;
	if (!fields[0].value.empty())
	{
		shaderInstance = database->getInstance(fields[0].value);
		if (!shaderInstance)
			return false;
	}

	Ref< render::ShaderGraph > sourceGraph = shaderInstance->getObject< render::ShaderGraph >();
	if (!sourceGraph)
		return false;

	Ref< db::Instance > outputInstance = group->createInstance(
		shaderName,
		db::CifReplaceExisting | db::CifKeepExistingGuid
	);
	if (!outputInstance)
		return false;

	outputInstance->setObject(sourceGraph);

	if (!outputInstance->commit())
		return false;

	return true;
}

	}
}
