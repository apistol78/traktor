/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/Editor/ImageProcess/ImageProcessShaderWizardTool.h"
#include "Render/Shader/ShaderGraph.h"
#include "Ui/Custom/InputDialog.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessShaderWizardTool", 0, ImageProcessShaderWizardTool, editor::IWizardTool)

std::wstring ImageProcessShaderWizardTool::getDescription() const
{
	return i18n::Text(L"RENDER_IMAGEPROCESS_SHADER_WIZARDTOOL_DESCRIPTION");
}

uint32_t ImageProcessShaderWizardTool::getFlags() const
{
	return WfGroup;
}

bool ImageProcessShaderWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	ui::custom::InputDialog inputDialog;
	ui::custom::InputDialog::Field fields[] =
	{
		ui::custom::InputDialog::Field(
			i18n::Text(L"RENDER_IMAGEPROCESS_SHADER_WIZARDTOOL_NAME")
		)
	};

	inputDialog.create(
		parent,
		i18n::Text(L"RENDER_IMAGEPROCESS_SHADER_WIZARDTOOL_DIALOG_TITLE"),
		i18n::Text(L"RENDER_IMAGEPROCESS_SHADER_WIZARDTOOL_DIALOG_MESSAGE"),
		fields,
		sizeof_array(fields)
	);

	if (inputDialog.showModal() != ui::DrOk)
	{
		inputDialog.destroy();
		return false;
	}

	inputDialog.destroy();

	Ref< render::ShaderGraph > sourceGraph = editor->getSourceDatabase()->getObjectReadOnly< render::ShaderGraph >(Guid(L"{B5537271-73A6-F545-A651-99D6DF10481B}"));
	if (!sourceGraph)
		return false;

	Ref< db::Instance > outputInstance = group->createInstance(
		fields[0].value,
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
