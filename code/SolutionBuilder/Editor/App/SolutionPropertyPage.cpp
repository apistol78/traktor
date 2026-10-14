/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <set>
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfmPrimitive.h"
#include "Core/System/OS.h"
#include "Core/System/ResolveEnv.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Editor/App/SolutionPropertyPage.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/Edit.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Static.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridColumnClickEvent.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridItemContentChangeEvent.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"

namespace traktor::sb
{
	namespace
	{

void scanEnvironmentVariables(const std::wstring& path, std::set< std::wstring >& outEnvs)
{
	size_t ofs = 0;
	for (;;)
	{
		const size_t s = path.find(L"$(", ofs);
		if (s == std::string::npos)
			break;

		const size_t e = path.find(L")", s + 2);
		if (e == std::string::npos)
			break;

		const std::wstring name = path.substr(s + 2, e - s - 2);
		if (!name.empty())
			outEnvs.insert(name);

		ofs = e + 1;
	}
}

void scanEnvironmentVariables(const ISerializable* object, std::set< const ISerializable* >& inoutVisited, std::set< std::wstring >& outEnvs)
{
	Ref< Reflection > reflection = Reflection::create(object);
	if (!reflection)
		return;

	RefArray< ReflectionMember > stringMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmPrimitiveWideString >()), stringMembers);
	for (auto member : stringMembers)
	{
		RfmPrimitiveWideString* stringMember = static_cast< RfmPrimitiveWideString* >(member.ptr());
		scanEnvironmentVariables(stringMember->get(), outEnvs);
	}

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);
	for (auto member : objectMembers)
	{
		RfmObject* objectMember = static_cast< RfmObject* >(member.ptr());
		if (objectMember->get() && inoutVisited.find(objectMember->get()) == inoutVisited.end())
		{
			inoutVisited.insert(objectMember->get());
			scanEnvironmentVariables(objectMember->get(), inoutVisited, outEnvs);
		}
	}
}

int32_t indexOfRow(const ui::GridView* grid, const ui::GridRow* row)
{
	const auto& rows = grid->getRows();
	for (int32_t i = 0; i < (int32_t)rows.size(); ++i)
		if (rows[i] == row)
			return i;
	return -1;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.SolutionPropertyPage", SolutionPropertyPage, ui::Container)

bool SolutionPropertyPage::create(ui::Widget* parent)
{
	if (!ui::Container::create(
		parent,
		ui::WsNone,
		new ui::TableLayout(L"100%", L"*,100%,100%", 4_ut, 8_ut)
	))
		return false;

	Ref< ui::Container > containerTop = new ui::Container();
	containerTop->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0_ut, 4_ut));

	Ref< ui::Static > rootLabel = new ui::Static();
	rootLabel->create(containerTop, L"Build path");

	m_rootPath = new ui::Edit();
	m_rootPath->create(containerTop);
	m_rootPath->addEventHandler< ui::FocusEvent >(this, &SolutionPropertyPage::eventEditFocus);

	Ref< ui::Static > staticAggregateOutputPath = new ui::Static();
	staticAggregateOutputPath->create(containerTop, L"Aggregate output path");

	m_aggregateOutputPath = new ui::Edit();
	m_aggregateOutputPath->create(containerTop);
	m_aggregateOutputPath->addEventHandler< ui::FocusEvent >(this, &SolutionPropertyPage::eventEditFocus);

	Ref< ui::Container > containerVariables = new ui::Container();
	containerVariables->create(this, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*", 0_ut, 4_ut));

	Ref< ui::Static > variablesLabel = new ui::Static();
	variablesLabel->create(containerVariables, L"Environment variables");

	m_gridVariables = new ui::GridView();
	m_gridVariables->create(containerVariables, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader | ui::GridView::WsAutoEdit);
	m_gridVariables->addColumn(new ui::GridColumn(L"Name", 160_ut, true));
	m_gridVariables->addColumn(new ui::GridColumn(L"Value", 400_ut, true));
	m_gridVariables->addColumn(new ui::GridColumn(L"Overwrite", 80_ut, false));
	m_gridVariables->addEventHandler< ui::GridItemContentChangeEvent >(this, &SolutionPropertyPage::eventVariableChange);
	m_gridVariables->addEventHandler< ui::GridColumnClickEvent >(this, &SolutionPropertyPage::eventVariableClick);

	Ref< ui::Container > containerButtons = new ui::Container();
	containerButtons->create(containerVariables, ui::WsNone, new ui::TableLayout(L"*,*", L"*", 0_ut, 4_ut));

	Ref< ui::Button > buttonAdd = new ui::Button();
	buttonAdd->create(containerButtons, L"Add");
	buttonAdd->addEventHandler< ui::ButtonClickEvent >(this, &SolutionPropertyPage::eventClickAddVariable);

	Ref< ui::Button > buttonRemove = new ui::Button();
	buttonRemove->create(containerButtons, L"Remove");
	buttonRemove->addEventHandler< ui::ButtonClickEvent >(this, &SolutionPropertyPage::eventClickRemoveVariable);

	Ref< ui::Container > containerBottom = new ui::Container();
	containerBottom->create(this, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 4_ut));

	Ref< ui::Static > environmentLabel = new ui::Static();
	environmentLabel->create(containerBottom, L"Referenced environment variables");

	m_environmentPreview = new ui::GridView();
	m_environmentPreview->create(containerBottom, ui::WsAccelerated | ui::GridView::WsColumnHeader);
	m_environmentPreview->addColumn(new ui::GridColumn(L"Name", 160_ut));
	m_environmentPreview->addColumn(new ui::GridColumn(L"Value", 480_ut));

	m_imageUnchecked = new ui::StyleBitmap(L"UI.Unchecked");
	m_imageChecked = new ui::StyleBitmap(L"UI.Checked");

	return true;
}

void SolutionPropertyPage::set(Solution* solution)
{
	m_solution = solution;

	m_rootPath->setText(m_solution->getRootPath());
	m_aggregateOutputPath->setText(m_solution->getAggregateOutputPath());
	updateVariables();
	updateReferencedEnvironment();

	m_rootPath->setFocus();
}

void SolutionPropertyPage::updateVariables()
{
	m_gridVariables->removeAllRows();

	for (const auto& variable : m_solution->getEnvironmentVariables())
	{
		Ref< ui::GridRow > row = new ui::GridRow();
		row->add(variable.name);
		row->add(variable.value);
		row->add(variable.overwrite ? m_imageChecked : m_imageUnchecked);
		m_gridVariables->addRow(row);
	}
}

void SolutionPropertyPage::updateReferencedEnvironment()
{
	m_environmentPreview->removeAllRows();

	std::set< std::wstring > referencedEnvs;
	if (m_solution != nullptr)
	{
		std::set< const ISerializable* > visited;
		scanEnvironmentVariables(m_solution, visited, referencedEnvs);
	}

	for (const auto& referencedEnv : referencedEnvs)
	{
		std::wstring value = L"";
		const bool defined = OS::getInstance().getEnvironment(referencedEnv, value);

		// Show value as builder would set it from solution.
		for (const auto& variable : m_solution->getEnvironmentVariables())
			if (variable.name == referencedEnv && (variable.overwrite || !defined))
				value = resolveEnv(variable.value, nullptr);

		Ref< ui::GridRow > row = new ui::GridRow();
		row->add(referencedEnv);
		row->add(value);
		m_environmentPreview->addRow(row);
	}
}

void SolutionPropertyPage::eventEditFocus(ui::FocusEvent* event)
{
	if (!event->lostFocus())
		return;

	m_solution->setRootPath(m_rootPath->getText());
	m_solution->setAggregateOutputPath(m_aggregateOutputPath->getText());
	updateReferencedEnvironment();
}

void SolutionPropertyPage::eventVariableChange(ui::GridItemContentChangeEvent* event)
{
	const ui::GridRow* row = event->getItem()->getRow();
	const int32_t index = indexOfRow(m_gridVariables, row);
	if (index < 0)
		return;

	AlignedVector< Solution::EnvironmentVariable > variables = m_solution->getEnvironmentVariables();
	variables[index].name = row->get(0)->getText();
	variables[index].value = row->get(1)->getText();
	m_solution->setEnvironmentVariables(variables);

	updateReferencedEnvironment();
	event->consume();
}

void SolutionPropertyPage::eventVariableClick(ui::GridColumnClickEvent* event)
{
	if (event->getColumn() != 2)
		return;

	const int32_t index = indexOfRow(m_gridVariables, event->getRow());
	if (index < 0)
		return;

	AlignedVector< Solution::EnvironmentVariable > variables = m_solution->getEnvironmentVariables();
	variables[index].overwrite = !variables[index].overwrite;
	m_solution->setEnvironmentVariables(variables);

	event->getRow()->set(2, variables[index].overwrite ? m_imageChecked : m_imageUnchecked);
	m_gridVariables->requestUpdate();

	updateReferencedEnvironment();
}

void SolutionPropertyPage::eventClickAddVariable(ui::ButtonClickEvent* event)
{
	AlignedVector< Solution::EnvironmentVariable > variables = m_solution->getEnvironmentVariables();
	variables.push_back({ L"NEW_VARIABLE", L"" });
	m_solution->setEnvironmentVariables(variables);

	updateVariables();
	updateReferencedEnvironment();
}

void SolutionPropertyPage::eventClickRemoveVariable(ui::ButtonClickEvent* event)
{
	const int32_t index = indexOfRow(m_gridVariables, m_gridVariables->getSelectedRow());
	if (index < 0)
		return;

	AlignedVector< Solution::EnvironmentVariable > variables = m_solution->getEnvironmentVariables();
	variables.erase(variables.begin() + index);
	m_solution->setEnvironmentVariables(variables);

	updateVariables();
	updateReferencedEnvironment();
}

}
