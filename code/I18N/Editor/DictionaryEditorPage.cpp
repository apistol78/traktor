#include "I18N/Editor/DictionaryEditorPage.h"
#include "I18N/Dictionary.h"
#include "Ui/Container.h"
#include "Ui/MethodHandler.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/InputDialog.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace i18n
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.i18n.DictionaryEditorPage", DictionaryEditorPage, editor::IEditorPage)

DictionaryEditorPage::DictionaryEditorPage(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool DictionaryEditorPage::create(ui::Container* parent, editor::IEditorPageSite* site)
{
	m_gridDictionary = new ui::custom::GridView();
	m_gridDictionary->create(parent, ui::custom::GridView::WsColumnHeader | ui::WsDoubleBuffer);
	m_gridDictionary->addColumn(new ui::custom::GridColumn(L"Id", 100));
	m_gridDictionary->addColumn(new ui::custom::GridColumn(L"Text", 400));
	m_gridDictionary->addDoubleClickEventHandler(ui::createMethodHandler(this, &DictionaryEditorPage::eventGridDoubleClick));
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

bool DictionaryEditorPage::setDataObject(db::Instance* instance, Object* data)
{
	m_dictionaryInstance = instance;

	if (!(m_dictionary = dynamic_type_cast< Dictionary* >(data)))
		return false;

	const std::map< std::wstring, std::wstring >& map = m_dictionary->get();
	for (std::map< std::wstring, std::wstring >::const_iterator i = map.begin(); i != map.end(); ++i)
	{
		Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
		row->addItem(new ui::custom::GridItem(i->first));
		row->addItem(new ui::custom::GridItem(i->second));
		m_gridDictionary->addRow(row);
	}
	m_gridDictionary->update();

	return true;
}

Ref< db::Instance > DictionaryEditorPage::getDataInstance()
{
	return m_dictionaryInstance;
}

Ref< Object > DictionaryEditorPage::getDataObject()
{
	return m_dictionary;
}

bool DictionaryEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool DictionaryEditorPage::handleCommand(const ui::Command& command)
{
	return false;
}

void DictionaryEditorPage::handleDatabaseEvent(const Guid& eventId)
{
}

void DictionaryEditorPage::eventGridDoubleClick(ui::Event* event)
{
	RefArray< ui::custom::GridRow > selectedRows;
	m_gridDictionary->getRows(selectedRows, ui::custom::GridView::GfSelectedOnly);

	if (selectedRows.size() > 0)
	{
		// Edit selected.
		ui::custom::InputDialog::Field fields[] =
		{
			{ L"Localized text", selectedRows[0]->getItems().at(1)->getText(), 0 }
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
			m_dictionary->set(selectedRows[0]->getItems().at(0)->getText(), fields->value);
			selectedRows[0]->getItems().at(1)->setText(fields->value);
			m_gridDictionary->update();
		}
		inputDialog.destroy();
	}
	else
	{
		// Add new.
		m_dictionary->set(L"", L"");

		Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
		row->addItem(new ui::custom::GridItem(L""));
		row->addItem(new ui::custom::GridItem(L""));
		m_gridDictionary->addRow(row);
		m_gridDictionary->update();
	}
}

	}
}
