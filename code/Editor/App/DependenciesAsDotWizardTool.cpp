/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Io/StringOutputStream.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/PipelineDependency.h"
#include "Editor/PipelineDependencySet.h"
#include "Editor/App/DependenciesAsDotWizardTool.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.DependenciesAsDotWizardToolTool", 0, DependenciesAsDotWizardTool, IWizardTool)

std::wstring DependenciesAsDotWizardTool::getDescription() const
{
	return i18n::Text(L"DEPENDENCIES_AS_DOT_WIZARDTOOL_DESCRIPTION");
}

const TypeInfoSet DependenciesAsDotWizardTool::getSupportedTypes() const
{
	return TypeInfoSet();
}

uint32_t DependenciesAsDotWizardTool::getFlags() const
{
	return WfInstance;
}

bool DependenciesAsDotWizardTool::launch(ui::Widget* parent, IEditor* editor, db::Group* group, db::Instance* instance)
{
	PipelineDependencySet dependencySet;
	Ref< IPipelineDepends > depends = editor->createPipelineDepends(&dependencySet, std::numeric_limits< uint32_t >::max());
	if (!depends)
		return false;

	depends->addDependency(instance, editor::PdfBuild);
	depends->waitUntilFinished();

	StringOutputStream ss;
	ss << L"digraph G {" << Endl;
	ss << IncreaseIndent;
	ss << L"node [shape=box];" << Endl;

	const uint32_t size = dependencySet.size();
	for (uint32_t i = 0; i < size; ++i)
	{
		const PipelineDependency* dependency = dependencySet.get(i);
		ss << L"\"" << dependency->outputGuid.format() << L"\" [label=\"" << dependency->outputPath << L"\"];" << Endl;
	}

	for (uint32_t i = 0; i < size; ++i)
	{
		const PipelineDependency* dependency = dependencySet.get(i);
		for (auto childIndex : dependency->children)
		{
			const PipelineDependency* child = dependencySet.get(childIndex);
			ss << L"\"" << dependency->outputGuid.format() << L"\" -> \"" << child->outputGuid.format() << L"\";" << Endl;
		}
		for (const auto& file : dependency->files)
			ss << L"\"" << dependency->outputGuid.format() << L"\" -> \"" << file.filePath.getOriginal() << L"\";" << Endl;
	}

	ss << DecreaseIndent;
	ss << L"}" << Endl;

	ui::Clipboard* clipboard = ui::Application::getInstance()->getClipboard();
	clipboard->setText(ss.str());

	return true;
}

}
