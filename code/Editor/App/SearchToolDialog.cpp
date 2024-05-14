/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <regex>
#include "Core/RefSet.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfmPrimitive.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "Editor/App/SearchToolDialog.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/CheckBox.h"
#include "Ui/Edit.h"
#include "Ui/FloodLayout.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/TableLayout.h"
#include "Ui/StyleBitmap.h"
#include "Ui/FileDialog.h"
#include "Ui/ProgressBar.h"
#include "Ui/Splitter.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

/*! Stylize member name.
 *
 * Transform from internal naming convention to
 * more human acceptable form.
 *
 * Example:
 * thisIsACommonName -> This is a common name
 *
 * \param memberName Name of member in internal naming convention.
 * \return Human acceptable form.
 */
std::wstring stylizeMemberName(const std::wstring& memberName)
{
	T_ASSERT(!memberName.empty());
	StringOutputStream ss;
	std::wstring::const_iterator i = memberName.begin();
	ss.put(toupper(*i++));
	for (; i != memberName.end(); ++i)
	{
		if (isupper(*i))
		{
			ss.put(L' ');
			ss.put(tolower(*i));
		}
		else
			ss.put(*i);
	}
	return ss.str();
}

std::wstring getMemberValue(const ReflectionMember* member)
{
	if (!member)
		return L"null";
	else if (const RfmPrimitiveBoolean* memberBool = dynamic_type_cast< const RfmPrimitiveBoolean* >(member))
		return memberBool->get() ? L"True" : L"False";
	else if (const RfmPrimitiveInt32* memberInt32 = dynamic_type_cast< const RfmPrimitiveInt32* >(member))
		return toString(memberInt32->get());
	else if (const RfmPrimitiveString* memberString = dynamic_type_cast< const RfmPrimitiveString* >(member))
		return mbstows(memberString->get());
	else if (const RfmPrimitiveWideString* memberWideString = dynamic_type_cast< const RfmPrimitiveWideString* >(member))
		return memberWideString->get();
	else if (const RfmPrimitivePath* memberPath = dynamic_type_cast< const RfmPrimitivePath* >(member))
		return memberPath->get().getPathName();
	else
		return L"";
}

bool match(const std::wstring& value, const std::wstring& needle, bool regExp, bool caseSensitive)
{
	if (!regExp)
	{
		StringSplit< std::wstring > ss(value, L"\n\r");
		if (caseSensitive)
		{
			for (auto s : ss)
			{
				if (s.empty())
					continue;
				if (s.find(needle) != std::wstring::npos)
					return true;
			}
		}
		else
		{
			for (auto s : ss)
			{
				if (s.empty())
					continue;
				if (toLower(s).find(toLower(needle)) != std::wstring::npos)
					return true;
			}
		}
	}
	else
	{
		StringSplit< std::wstring > ss(value, L"\n\r");
		if (caseSensitive)
		{
			std::basic_regex< wchar_t > rx(needle);
			for (auto s : ss)
			{
				if (s.empty())
					continue;
				if (std::regex_match(s, rx))
					return true;
			}
		}
		else
		{
			std::basic_regex< wchar_t > rx(needle);
			for (auto s : ss)
			{
				if (s.empty())
					continue;
				if (std::regex_match(toLower(s), rx))
					return true;
			}
		}
	}
	return false;
}

Ref< const ReflectionMember > searchMember(db::Instance* instance, Reflection* reflection, const ReflectionMember* member, RefSet< Object >& visited, const std::wstring& needle, bool regExp, bool caseSensitive, ui::GridView* gridResults)
{
	if (match(stylizeMemberName(member->getName()), needle, regExp, caseSensitive))
		return member;
	else if (const RfmCompound* memberCompound = dynamic_type_cast< const RfmCompound* >(member))
	{
		const RefArray< ReflectionMember >& members = memberCompound->getMembers();
		for (RefArray< ReflectionMember >::const_iterator i = members.begin(); i != members.end(); ++i)
		{
			Ref< const ReflectionMember > foundMember = searchMember(instance, reflection, *i, visited, needle, regExp, caseSensitive, gridResults);
			if (foundMember)
				return foundMember;
		}
	}
	else if (const RfmObject* memberObject = dynamic_type_cast< const RfmObject* >(member))
	{
		if (memberObject->get() && visited.insert(memberObject->get()))
		{
			Ref< Reflection > childReflection = Reflection::create(memberObject->get());
			if (childReflection)
			{
				const RefArray< ReflectionMember >& members = childReflection->getMembers();
				for (RefArray< ReflectionMember >::const_iterator i = members.begin(); i != members.end(); ++i)
				{
					Ref< const ReflectionMember > foundMember = searchMember(instance, childReflection, *i, visited, needle, regExp, caseSensitive, gridResults);
					if (foundMember)
						return foundMember;
				}
			}
		}
	}
	else if (const RfmPrimitiveString* memberString = dynamic_type_cast< const RfmPrimitiveString* >(member))
	{
		if (match(mbstows(memberString->get()), needle, regExp, caseSensitive))
			return member;
	}
	else if (const RfmPrimitiveWideString* memberWideString = dynamic_type_cast< const RfmPrimitiveWideString* >(member))
	{
		if (match(memberWideString->get(), needle, regExp, caseSensitive))
			return member;
	}
	else if (const RfmPrimitivePath* memberPath = dynamic_type_cast< const RfmPrimitivePath* >(member))
	{
		if (match(memberPath->get().getPathName(), needle, regExp, caseSensitive))
			return member;
	}

	return 0;
}

void searchInstance(db::Instance* instance, const std::wstring& needle, bool regExp, bool caseSensitive, ui::GridView* gridResults)
{
	Ref< ISerializable > object = instance->getObject();
	if (!object)
		return;

	Ref< Reflection > reflection = Reflection::create(object);
	if (!reflection)
		return;

	RefSet< Object > visited;

	const RefArray< ReflectionMember >& members = reflection->getMembers();
	for (RefArray< ReflectionMember >::const_iterator i = members.begin(); i != members.end(); ++i)
	{
		Ref< const ReflectionMember > foundMember = searchMember(instance, reflection, *i, visited, needle, regExp, caseSensitive, gridResults);
		if (foundMember)
		{
			std::wstring value = getMemberValue(foundMember);

			Ref< ui::GridRow > row = new ui::GridRow();
			row->add(instance->getPath());
			row->add(instance->getPrimaryType()->getName());
			row->add(stylizeMemberName(foundMember->getName()));

			if (value.find_first_of(L"\n\r") == value.npos)
				row->add(getMemberValue(foundMember));
			else
				row->add(L"...");

			row->setData(L"INSTANCE", instance);
			gridResults->addRow(row);
		}
	}
}

void searchGroup(db::Group* group, const std::wstring& needle, bool regExp, bool caseSensitive, ui::ProgressBar* progressBar, ui::GridView* gridResults)
{
	RefArray< db::Instance > childInstances;
	db::recursiveFindChildInstances(group, db::FindInstanceAll(), childInstances);

	progressBar->setRange(0, int32_t(childInstances.size()));
	for (int32_t i = 0; i < int32_t(childInstances.size()); ++i)
	{
		searchInstance(childInstances[i], needle, regExp, caseSensitive, gridResults);
		progressBar->setProgress(i);
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.SearchToolDialog", SearchToolDialog, ui::Dialog)

SearchToolDialog::SearchToolDialog(IEditor* editor)
:	m_editor(editor)
{
}

void SearchToolDialog::destroy()
{
	if (m_jobSearch)
		m_jobSearch->wait();

	ui::Dialog::destroy();
}

bool SearchToolDialog::create(ui::Widget* parent)
{
	if (!ui::Dialog::create(
		parent,
		i18n::Text(L"EDITOR_SEARCH_TOOL_TITLE"),
		1100_ut,
		600_ut,
		ui::Dialog::WsCenterParent | ui::Dialog::WsDefaultResizable,
		new ui::FloodLayout()
	))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	Ref< ui::Splitter > splitterV = new ui::Splitter();
	splitterV->create(this, true, 220_ut, false);

	Ref< ui::Container > containerSearch = new ui::Container();
	containerSearch->create(splitterV, ui::WsNone, new ui::TableLayout(L"100%", L"*", 4_ut, 4_ut));

	m_editSearch = new ui::Edit();
	m_editSearch->create(containerSearch, L"", ui::WsWantAllInput);
	m_editSearch->addEventHandler< ui::KeyDownEvent >(this, &SearchToolDialog::eventSearchKey);

	m_checkRegExp = new ui::CheckBox();
	m_checkRegExp->create(containerSearch, i18n::Text(L"EDITOR_SEARCH_TOOL_REGEXP"));

	m_checkCaseSensitive = new ui::CheckBox();
	m_checkCaseSensitive->create(containerSearch, i18n::Text(L"EDITOR_SEARCH_TOOL_CASE_SENSITIVE"));

	m_buttonFind = new ui::Button();
	m_buttonFind->create(containerSearch, i18n::Text(L"EDITOR_SEARCH_TOOL_FIND"));
	m_buttonFind->addEventHandler< ui::ButtonClickEvent >(this, &SearchToolDialog::eventButtonSearchClick);

	m_buttonSaveAs = new ui::Button();
	m_buttonSaveAs->create(containerSearch, i18n::Text(L"EDITOR_SEARCH_TOOL_SAVE_AS"));
	m_buttonSaveAs->addEventHandler< ui::ButtonClickEvent >(this, &SearchToolDialog::eventButtonSaveAsClick);

	m_progressBar = new ui::ProgressBar();
	m_progressBar->create(containerSearch, ui::WsDoubleBuffer);
	m_progressBar->setVisible(false);

	m_gridResults = new ui::GridView();
	m_gridResults->create(splitterV, ui::GridView::WsColumnHeader | ui::WsDoubleBuffer);
	m_gridResults->addColumn(new ui::GridColumn(i18n::Text(L"EDITOR_SEARCH_TOOL_INSTANCE"), 320_ut));
	m_gridResults->addColumn(new ui::GridColumn(i18n::Text(L"EDITOR_SEARCH_TOOL_TYPE"), 200_ut));
	m_gridResults->addColumn(new ui::GridColumn(i18n::Text(L"EDITOR_SEARCH_TOOL_MEMBER"), 200_ut));
	m_gridResults->addColumn(new ui::GridColumn(i18n::Text(L"EDITOR_SEARCH_TOOL_VALUE"), 200_ut));
	m_gridResults->addEventHandler< ui::MouseDoubleClickEvent >(this, &SearchToolDialog::eventGridResultDoubleClick);
	m_gridResults->addEventHandler< ui::MouseButtonUpEvent >(this, &SearchToolDialog::eventGridResultButtonUp);

	addEventHandler< ui::TimerEvent >(this, &SearchToolDialog::eventTimer);
	addEventHandler< ui::CloseEvent >(this, &SearchToolDialog::eventClose);

	update();

	return true;
}

void SearchToolDialog::show()
{
	ui::Dialog::show();
	m_editSearch->setFocus();
}

void SearchToolDialog::search(const std::wstring& needle)
{
	bool regExp = m_checkRegExp->isChecked();
	bool caseSensitive = m_checkCaseSensitive->isChecked();

	m_gridResults->removeAllRows();
	m_jobSearch = JobManager::getInstance().add([=, this](){
		jobSearch(needle, regExp, caseSensitive);
	});

	m_editSearch->setEnable(false);
	m_checkRegExp->setEnable(false);
	m_checkCaseSensitive->setEnable(false);
	m_buttonFind->setEnable(false);
	m_buttonSaveAs->setEnable(false);
	m_progressBar->setVisible(true);

	startTimer(100);
}

void SearchToolDialog::eventSearchKey(ui::KeyDownEvent* event)
{
	if (m_editSearch->isEnable(true) && event->getVirtualKey() == ui::VkReturn)
	{
		std::wstring needle = m_editSearch->getText();
		if (!needle.empty())
			search(needle);
	}
}

void SearchToolDialog::eventButtonSearchClick(ui::ButtonClickEvent* event)
{
	std::wstring needle = m_editSearch->getText();
	if (!needle.empty())
		search(needle);
}

void SearchToolDialog::eventButtonSaveAsClick(ui::ButtonClickEvent* event)
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), i18n::Text(L"EDITOR_SEARCH_TOOL_SAVE_AS_TITLE"), L"All files;*.*", L"", true))
		return;

	Path saveAsPath;
	if (fileDialog.showModal(saveAsPath) != ui::DialogResult::Ok)
		return;

	fileDialog.destroy();

	Ref< IStream > file = FileSystem::getInstance().open(saveAsPath, File::FmWrite);
	if (!file)
		return;

	Utf8Encoding encoding;
	FileOutputStream fos(file, &encoding);

	const RefArray< ui::GridRow >& rows = m_gridResults->getRows();
	for (RefArray< ui::GridRow >::const_iterator i = rows.begin(); i != rows.end(); ++i)
	{
		fos <<
			(*i)->get(0)->getText() << L";" <<
			(*i)->get(1)->getText() << L";" <<
			(*i)->get(2)->getText() << L";" <<
			(*i)->get(3)->getText() <<
		Endl;
	}

	fos.close();
}

void SearchToolDialog::eventGridResultDoubleClick(ui::MouseDoubleClickEvent* event)
{
	Ref< ui::GridRow > row = m_gridResults->getSelectedRow();
	if (row)
	{
		Ref< db::Instance > instance = row->getData< db::Instance >(L"INSTANCE");
		T_ASSERT(instance);

		m_editor->openEditor(instance);
	}
}

void SearchToolDialog::eventGridResultButtonUp(ui::MouseButtonUpEvent* event)
{
	Ref< ui::GridRow > row = m_gridResults->getSelectedRow();
	if (row && event->getButton() == ui::MbtRight)
	{
		Ref< db::Instance > instance = row->getData< db::Instance >(L"INSTANCE");
		T_ASSERT(instance);

		Ref< ui::Menu > popupMenu = new ui::Menu();
		popupMenu->add(new ui::MenuItem(ui::Command(L"SearchTool.OpenInstance"), i18n::Text(L"EDITOR_SEARCH_TOOL_OPEN_INSTANCE")));
		popupMenu->add(new ui::MenuItem(ui::Command(L"SearchTool.HighlightInstance"), i18n::Text(L"EDITOR_SEARCH_TOOL_HIGHLIGHT_INSTANCE")));

		const ui::MenuItem* selectedItem = popupMenu->showModal(m_gridResults, event->getPosition());
		if (selectedItem)
		{
			if (selectedItem->getCommand() == L"SearchTool.OpenInstance")
				m_editor->openEditor(instance);
			else if (selectedItem->getCommand() == L"SearchTool.HighlightInstance")
				m_editor->highlightInstance(instance);
		}
	}
}

void SearchToolDialog::eventTimer(ui::TimerEvent* event)
{
	if (m_jobSearch->wait(0))
	{
		stopTimer();
		m_editSearch->setEnable(true);
		m_checkRegExp->setEnable(true);
		m_checkCaseSensitive->setEnable(true);
		m_buttonFind->setEnable(true);
		m_buttonSaveAs->setEnable(true);
		m_progressBar->setProgress(0);
		m_progressBar->setVisible(false);
		update();
	}
}

void SearchToolDialog::eventClose(ui::CloseEvent* event)
{
	destroy();
}

void SearchToolDialog::jobSearch(std::wstring needle, bool regExp, bool caseSensitive)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
		return;

	searchGroup(
		database->getRootGroup(),
		needle,
		regExp,
		caseSensitive,
		m_progressBar,
		m_gridResults
	);
}

	}
}
