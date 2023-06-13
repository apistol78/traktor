/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "I18N/Dictionary.h"
#include "I18N/Text.h"
#include "I18N/Editor/DictionaryEditorPage.h"
#include "I18N/Editor/IDictionaryFormat.h"
#include "I18N/Editor/Translator.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Container.h"
#include "Ui/TableLayout.h"
#include "Ui/FileDialog.h"
#include "Ui/InputDialog.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridItemContentChangeEvent.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridRowDoubleClickEvent.h"
#include "Ui/GridView/GridView.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor
{
	namespace i18n
	{
		namespace
		{

const wchar_t* c_languages[] =
{
	L"Swedish", L"sv",
	L"Afrikaans", L"af",
	L"Albanian", L"sq",
	L"Arabic", L"ar",
	L"Armenian", L"hy",
	L"Azerbaijani", L"az",
	L"Basque", L"eu",
	L"Bengali; bangla", L"bn",
	L"Bosnian", L"bs",
	L"Bulgarian", L"bg",
	L"Cebuano", L"ceb",
	L"Czech", L"cs",
	L"Danish", L"da",
	L"Dutch", L"nl",
	L"English", L"en",
	L"Esperanto", L"eo",
	L"Estonian", L"et",
	L"Filipino", L"tl",
	L"Finnish", L"fi",
	L"French", L"fr",
	L"Galician", L"gl",
	L"Georgian", L"ka",
	L"German", L"de",
	L"Greek", L"el",
	L"Gujarati", L"gu",
	L"Haitian", L"ht",
	L"Hebrew", L"iw",
	L"Hindi", L"hi",
	L"Hmong", L"hmn",
	L"Indonesian", L"id",
	L"Irish", L"ga",
	L"Icelandic", L"is",
	L"Italian", L"it",
	L"Japanese", L"ja",
	L"Javanese", L"jw",
	L"Yiddish", L"yi",
	L"kanaresiska", L"kn",
	L"Catalan", L"ca",
	L"Khmer", L"km",
	L"Chinese", L"zh-CN",
	L"Korean", L"ko",
	L"Croatian", L"hr",
	L"Lao", L"lo",
	L"Latin", L"la",
	L"Latvian", L"lv",
	L"Lithuanian", L"lt",
	L"Macedonian", L"mk",
	L"Malay", L"ms",
	L"Maltese", L"mt",
	L"Marathi", L"mr",
	L"Norwegian", L"no",
	L"Persian", L"fa",
	L"Polish", L"pl",
	L"Portuguese", L"pt",
	L"Romanian", L"ro",
	L"Russian", L"ru",
	L"Serbian", L"sr",
	L"Slovak", L"sk",
	L"Slovenian", L"sl",
	L"Spanish", L"es",
	L"Swahili", L"sw",
	L"Tamil", L"ta",
	L"telugu", L"te",
	L"Thai", L"th",
	L"Turkish", L"tr",
	L"Ukrainian", L"uk",
	L"Hungarian", L"hu",
	L"Urdu", L"ur",
	L"Vietnamese", L"vi",
	L"Belarusian", L"be",
	L"Welsh", L"cy",
	0
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.i18n.DictionaryEditorPage", DictionaryEditorPage, editor::IEditorPage)

DictionaryEditorPage::DictionaryEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool DictionaryEditorPage::create(ui::Container* parent)
{
	m_dictionary = m_document->getObject< Dictionary >(0);
	if (!m_dictionary)
		return false;

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut));

	Ref< ui::ToolBar > toolBar = new ui::ToolBar();
	toolBar->create(container);
	toolBar->addItem(new ui::ToolBarButton(Text(L"DICTIONARY_EDITOR_IMPORT"), ui::Command(L"I18N.Editor.Import")));
	toolBar->addItem(new ui::ToolBarButton(Text(L"DICTIONARY_EDITOR_EXPORT"), ui::Command(L"I18N.Editor.Export")));
	toolBar->addItem(new ui::ToolBarButton(Text(L"DICTIONARY_EDITOR_TRANSLATE"), ui::Command(L"I18N.Editor.Translate")));
	toolBar->addItem(new ui::ToolBarButton(Text(L"DICTIONARY_EDITOR_COPY_UNIQUE_CHARS"), ui::Command(L"I18N.Editor.CopyUniqueChars")));
	toolBar->addItem(new ui::ToolBarButton(Text(L"DICTIONARY_EDITOR_SELECT_REFERENCE_DICTIONARY"), ui::Command(L"I18N.Editor.SelectReferenceDictionary")));
	toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &DictionaryEditorPage::eventToolClick);

	m_gridDictionary = new ui::GridView();
	m_gridDictionary->create(container, ui::GridView::WsColumnHeader | ui::GridView::WsMultiSelect | ui::WsDoubleBuffer);
	m_gridDictionary->addColumn(new ui::GridColumn(Text(L"DICTIONARY_EDITOR_COLUMN_ID"), 300_ut));
	m_gridDictionary->addColumn(new ui::GridColumn(Text(L"DICTIONARY_EDITOR_COLUMN_TEXT"), 600_ut));
	m_gridDictionary->addColumn(new ui::GridColumn(Text(L"DICTIONARY_EDITOR_COLUMN_REFERENCE"), 600_ut));
	m_gridDictionary->addEventHandler< ui::GridRowDoubleClickEvent >(this, &DictionaryEditorPage::eventGridRowDoubleClick);
	m_gridDictionary->addEventHandler< ui::GridItemContentChangeEvent >(this, &DictionaryEditorPage::eventGridItemChange);

	updateGrid();
	return true;
}

void DictionaryEditorPage::destroy()
{
	m_gridDictionary->destroy();
}

bool DictionaryEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool DictionaryEditorPage::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.Delete")
	{
		RefArray< ui::GridRow > selectedRows = m_gridDictionary->getRows(ui::GridView::GfSelectedOnly);
		if (!selectedRows.empty())
		{
			m_document->push();
			for (auto selectedRow : selectedRows)
			{
				m_dictionary->remove(selectedRow->get(0)->getText());
				m_gridDictionary->removeRow(selectedRow);
			}
		}
	}
	else
		return false;

	return true;
}

void DictionaryEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void DictionaryEditorPage::updateGrid()
{
	m_gridDictionary->removeAllRows();

	// Add all entries from dictionary.
	if (m_dictionary)
	{
		for (const auto& map : m_dictionary->get())
		{
			Ref< ui::GridRow > row = new ui::GridRow();
			row->add(new ui::GridItem(map.first));
			row->add(new ui::GridItem(map.second));

			if (m_referenceDictionary)
			{
				std::wstring referenceText;
				if (m_referenceDictionary->get(map.first, referenceText))
					row->add(new ui::GridItem(referenceText));
			}

			m_gridDictionary->addRow(row);
		}
	}

	// Add last empty row; double click on this and a new entry is added.
	Ref< ui::GridRow > row = new ui::GridRow();
	row->add(new ui::GridItem(Text(L"DICTIONARY_EDITOR_COLUMN_ADD")));
	row->add(new ui::GridItem(L""));
	m_gridDictionary->addRow(row);

	m_gridDictionary->update();
}

void DictionaryEditorPage::eventToolClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& cmd = event->getCommand();

	if (cmd == L"I18N.Editor.Import")
	{
		std::wstring line;

		ui::FileDialog fileDialog;
		if (!fileDialog.create(m_gridDictionary, type_name(this), Text(L"DICTIONARY_EDITOR_IMPORT_DICTIONARY_DIALOG"), L"All files;*.*"))
			return;

		Path fileName;
		if (fileDialog.showModal(fileName) != ui::DialogResult::Ok)
		{
			fileDialog.destroy();
			return;
		}
		fileDialog.destroy();

		Ref< Dictionary > dictionary = IDictionaryFormat::readAny(fileName, 0, 1);
		if (!dictionary)
			return;

		for (const auto& map : dictionary->get())
			m_dictionary->set(map.first, map.second);

		updateGrid();
	}
	else if (cmd == L"I18N.Editor.Export")
	{
		ui::FileDialog fileDialog;
		if (!fileDialog.create(m_gridDictionary, type_name(this), Text(L"DICTIONARY_EDITOR_EXPORT_DICTIONARY_DIALOG"), L"All files;*.*", L"", true))
			return;

		Path fileName;
		if (fileDialog.showModal(fileName) != ui::DialogResult::Ok)
		{
			fileDialog.destroy();
			return;
		}
		fileDialog.destroy();

		IDictionaryFormat::writeAny(fileName, m_dictionary);
	}
	else if (cmd == L"I18N.Editor.Translate")
	{
		ui::InputDialog::Field fields[] =
		{
			ui::InputDialog::Field(
				Text(L"DICTIONARY_EDITOR_FROM_LANGUAGE"),
				L"",
				nullptr,
				new ui::InputDialog::KeyValueArrayEnumerator(c_languages)
			),
			ui::InputDialog::Field(
				Text(L"DICTIONARY_EDITOR_TO_LANGUAGE"),
				L"",
				nullptr,
				new ui::InputDialog::KeyValueArrayEnumerator(c_languages)
			)
		};

		ui::InputDialog inputDialog;
		inputDialog.create(
			m_gridDictionary,
			Text(L"DICTIONARY_EDITOR_TRANSLATE"),
			Text(L"DICTIONARY_EDITOR_AUTO_TRANSLATE_WORDS"),
			fields,
			sizeof_array(fields)
		);
		if (inputDialog.showModal() == ui::DialogResult::Ok)
		{
			m_document->push();

			Translator translator(fields[0].value, fields[1].value);
			for (auto selectedRow : m_gridDictionary->getRows(ui::GridView::GfSelectedOnly))
			{
				std::wstring source = selectedRow->get(1)->getText();
				std::wstring out;

				if (translator.translate(source, out))
				{
					m_dictionary->set(
						selectedRow->get(0)->getText(),
						out
					);
					selectedRow->get(1)->setText(out);
				}
			}

			m_gridDictionary->update();
		}
		inputDialog.destroy();
	}
	else if (cmd == L"I18N.Editor.CopyUniqueChars")
	{
		std::set< wchar_t > uc;

		for (const auto& map : m_dictionary->get())
			uc.insert(map.second.begin(), map.second.end());

		StringOutputStream ss;
		for (auto ch : uc)
			ss << ch;

		Ref< ui::Clipboard > clipboard = ui::Application::getInstance()->getClipboard();
		if (clipboard)
			clipboard->setText(ss.str());

		log::debug << uc.size() << L" unique character(s) copied into clipboard" << Endl;
	}
	else if (cmd == L"I18N.Editor.SelectReferenceDictionary")
	{
		Ref< db::Instance > referenceInstance = m_editor->browseInstance(type_of< Dictionary >());
		if (referenceInstance)
		{
			m_referenceDictionary = referenceInstance->getObject< Dictionary >();
			updateGrid();
		}
	}
}

void DictionaryEditorPage::eventGridRowDoubleClick(ui::GridRowDoubleClickEvent* event)
{
	ui::GridRow* row = event->getRow();
	T_ASSERT(row);

	if (row != m_gridDictionary->getRows().back())
	{
		if (event->getColumnIndex() != 1)
			return;

		ui::GridItem* cell = row->get(event->getColumnIndex());
		T_ASSERT(cell);

		cell->edit();
	}
	else
	{
		// Add new last empty row; double click on this and a new entry is added.
		Ref< ui::GridRow > lastRow = new ui::GridRow();
		lastRow->add(new ui::GridItem(Text(L"DICTIONARY_EDITOR_COLUMN_ADD")));
		lastRow->add(new ui::GridItem(L""));
		m_gridDictionary->addRow(lastRow);

		// Modify former last row into being a new row, begin edit id.
		row->get(0)->setText(L"ID_");
		row->get(0)->edit();
	}
}

void DictionaryEditorPage::eventGridItemChange(ui::GridItemContentChangeEvent* event)
{
	ui::GridRow* row = event->getItem()->getRow();
	T_ASSERT(row);

	m_document->push();
	m_dictionary->set(
		row->get(0)->getText(),
		row->get(1)->getText()
	);

	event->consume();
}

	}
}
