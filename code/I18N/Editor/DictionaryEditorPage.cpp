#include "Core/Io/IStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Editor/IDocument.h"
#include "I18N/Dictionary.h"
#include "I18N/Editor/DictionaryEditorPage.h"
#include "I18N/Editor/Translator.h"
#include "Ui/Container.h"
#include "Ui/FileDialog.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/InputDialog.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"

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
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	Ref< ui::custom::ToolBar > toolBar = new ui::custom::ToolBar();
	toolBar->create(container);
	toolBar->addItem(new ui::custom::ToolBarButton(L"Import...", ui::Command(L"I18N.Editor.Import")));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Export...", ui::Command(L"I18N.Editor.Export")));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Translate...", ui::Command(L"I18N.Editor.Translate")));
	toolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &DictionaryEditorPage::eventToolClick);

	m_gridDictionary = new ui::custom::GridView();
	m_gridDictionary->create(container, ui::custom::GridView::WsColumnHeader | ui::WsDoubleBuffer);
	m_gridDictionary->addColumn(new ui::custom::GridColumn(L"Id", 300));
	m_gridDictionary->addColumn(new ui::custom::GridColumn(L"Text", 800));
	m_gridDictionary->addEventHandler< ui::MouseDoubleClickEvent >(this, &DictionaryEditorPage::eventGridDoubleClick);

	const std::map< std::wstring, std::wstring >& map = m_dictionary->get();
	for (std::map< std::wstring, std::wstring >::const_iterator i = map.begin(); i != map.end(); ++i)
	{
		Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(i->first));
		row->add(new ui::custom::GridItem(i->second));
		m_gridDictionary->addRow(row);
	}
	m_gridDictionary->update();

	return true;
}

void DictionaryEditorPage::destroy()
{
	m_gridDictionary->destroy();
}

void DictionaryEditorPage::activate()
{
}

void DictionaryEditorPage::deactivate()
{
}

bool DictionaryEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool DictionaryEditorPage::handleCommand(const ui::Command& command)
{
	return false;
}

void DictionaryEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void DictionaryEditorPage::eventToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& cmd = event->getCommand();

	if (cmd == L"I18N.Editor.Import")
	{
		std::wstring line;

		ui::FileDialog fileDialog;
		if (!fileDialog.create(m_gridDictionary, L"Import dictionary...", L"All files;*.*"))
			return;

		Path fileName;
		if (fileDialog.showModal(fileName) != ui::DrOk)
		{
			fileDialog.destroy();
			return;
		}
		fileDialog.destroy();

		// Assume CSV format.
		Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
		if (!file)
			return;

		StringReader sr(file, new Utf8Encoding());
		while (sr.readLine(line) >= 0)
		{
			std::vector< std::wstring > columns;

			uint32_t i0 = 0;
			bool quote = false;

			for (uint32_t i = 0; i < line.length(); ++i)
			{
				if (line[i] == L',' && quote == false)
				{
					std::wstring tmp = trim(line.substr(i0, i - i0));
					if (tmp.length() >= 2 && tmp[0] == L'\"' && tmp[tmp.length() - 1] == L'\"')
						tmp = tmp.substr(1, tmp.length() - 2);
					columns.push_back(tmp);
					i0 = i + 1;
				}
				if (line[i] == L'\"')
				{
					quote = !quote;
				}
			}

			if (!quote)
			{
				std::wstring tmp = trim(line.substr(i0));
				if (tmp.length() >= 2 && tmp[0] == L'\"' && tmp[tmp.length() - 1] == L'\"')
					tmp = tmp.substr(1, tmp.length() - 2);
				columns.push_back(tmp);
			}

			Split< std::wstring >::any(line, L",", columns, true);
			if (columns.size() >= 2 && !columns[0].empty())
			{
				m_dictionary->set(
					columns[0],
					columns[1]
				);
			}
		}

		file->close();
		file = 0;

		m_gridDictionary->removeAllRows();

		const std::map< std::wstring, std::wstring >& map = m_dictionary->get();
		for (std::map< std::wstring, std::wstring >::const_iterator i = map.begin(); i != map.end(); ++i)
		{
			Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
			row->add(new ui::custom::GridItem(i->first));
			row->add(new ui::custom::GridItem(i->second));
			m_gridDictionary->addRow(row);
		}
		m_gridDictionary->update();
	}
	else if (cmd == L"I18N.Editor.Export")
	{
		ui::FileDialog fileDialog;
		if (!fileDialog.create(m_gridDictionary, L"Export dictionary...", L"All files;*.*", true))
			return;

		Path fileName;
		if (fileDialog.showModal(fileName) != ui::DrOk)
		{
			fileDialog.destroy();
			return;
		}
		fileDialog.destroy();

		// Assume CSV format.
		Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
		if (!file)
			return;

		FileOutputStream fos(file, new Utf8Encoding());

		const std::map< std::wstring, std::wstring >& map = m_dictionary->get();
		for (std::map< std::wstring, std::wstring >::const_iterator i = map.begin(); i != map.end(); ++i)
			fos << i->first << L"," << i->second << Endl;

		fos.close();
	}
	else if (cmd == L"I18N.Editor.Translate")
	{
		ui::custom::InputDialog::Field fields[] =
		{
			{
				L"From language",
				L"",
				0,
				c_languages
			},
			{
				L"To language",
				L"",
				0,
				c_languages
			}
		};

		ui::custom::InputDialog inputDialog;
		inputDialog.create(
			m_gridDictionary,
			L"Translate",
			L"Automatic translate word(s)",
			fields,
			sizeof_array(fields)
		);
		if (inputDialog.showModal() == ui::DrOk)
		{
			m_document->push();

			RefArray< ui::custom::GridRow > selectedRows;
			m_gridDictionary->getRows(selectedRows, ui::custom::GridView::GfSelectedOnly);

			Translator translator(fields[0].value, fields[1].value);

			for (RefArray< ui::custom::GridRow >::iterator i = selectedRows.begin(); i != selectedRows.end(); ++i)
			{
				std::wstring source = checked_type_cast< ui::custom::GridItem*, false >((*i)->get(1))->getText();
				std::wstring out;

				if (translator.translate(source, out))
				{
					m_dictionary->set(
						checked_type_cast< ui::custom::GridItem*, false >((*i)->get(0))->getText(),
						out
					);
					checked_type_cast< ui::custom::GridItem*, false >((*i)->get(1))->setText(out);
				}
			}

			m_gridDictionary->update();
		}
		inputDialog.destroy();
	}
}

void DictionaryEditorPage::eventGridDoubleClick(ui::MouseDoubleClickEvent* event)
{
	RefArray< ui::custom::GridRow > selectedRows;
	m_gridDictionary->getRows(selectedRows, ui::custom::GridView::GfSelectedOnly);

	if (selectedRows.size() > 0)
	{
		ui::custom::InputDialog::Field fields[] =
		{
			{
				L"Localized text",
				checked_type_cast< ui::custom::GridItem*, false >(selectedRows[0]->get(1))->getText(),
				0,
				0
			}
		};

		ui::custom::InputDialog inputDialog;
		inputDialog.create(
			m_gridDictionary,
			L"Enter localization",
			L"Enter localization string",
			fields,
			sizeof_array(fields)
		);
		if (inputDialog.showModal() == ui::DrOk)
		{
			m_document->push();
			m_dictionary->set(
				checked_type_cast< ui::custom::GridItem*, false >(selectedRows[0]->get(0))->getText(),
				fields->value
			);

			checked_type_cast< ui::custom::GridItem*, false >(selectedRows[0]->get(1))->setText(fields->value);
			m_gridDictionary->update();
		}
		inputDialog.destroy();
	}
	else
	{
		m_document->push();
		m_dictionary->set(L"", L"");

		Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(L""));
		row->add(new ui::custom::GridItem(L""));
		m_gridDictionary->addRow(row);
		m_gridDictionary->update();
	}
}

	}
}
