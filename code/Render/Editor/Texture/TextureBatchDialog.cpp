#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Editor/Texture/TextureBatchDialog.h"
#include "Ui/Bitmap.h"
#include "Ui/FileDialog.h"
#include "Ui/FloodLayout.h"
#include "Ui/TableLayout.h"
#include "Ui/ListBox.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"

// Resources
#include "Resources/PlusMinus.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureBatchDialog", TextureBatchDialog, ui::ConfigDialog)

TextureBatchDialog::TextureBatchDialog(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool TextureBatchDialog::create(ui::Widget* parent)
{
	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"TEXTURE_BATCH_DIALOG_TITLE"),
		900,
		500,
		ui::ConfigDialog::WsDefaultResizable,
		new ui::FloodLayout()
	))
		return false;

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, true, 200);

	Ref< ui::Container > textureListContainer = new ui::Container();
	textureListContainer->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	Ref< ui::custom::ToolBar > textureListTools = new ui::custom::ToolBar();
	if (!textureListTools->create(textureListContainer))
		return false;

	textureListTools->addImage(ui::Bitmap::load(c_ResourcePlusMinus, sizeof(c_ResourcePlusMinus), L"png"), 4);
	textureListTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"TEXTURE_BATCH_ADD"), 0, ui::Command(L"TextureBatch.Add")));
	textureListTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"TEXTURE_BATCH_REMOVE"), 1, ui::Command(L"TextureBatch.Remove")));
	textureListTools->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &TextureBatchDialog::eventTextureListToolClick);

	m_textureList = new ui::ListBox();
	m_textureList->create(textureListContainer, L"", ui::WsClientBorder | ui::ListBox::WsExtended);
	m_textureList->addEventHandler< ui::SelectionChangeEvent >(this, &TextureBatchDialog::eventTextureListSelect);

	m_texturePropertyList = new ui::custom::AutoPropertyList();
	m_texturePropertyList->create(splitter, ui::WsClientBorder | ui::WsDoubleBuffer | ui::custom::AutoPropertyList::WsColumnHeader);
	m_texturePropertyList->setSeparator(200);
	m_texturePropertyList->setColumnName(0, i18n::Text(L"PROPERTY_COLUMN_NAME"));
	m_texturePropertyList->setColumnName(1, i18n::Text(L"PROPERTY_COLUMN_VALUE"));

	return true;
}

void TextureBatchDialog::destroy()
{
	ui::ConfigDialog::destroy();
}

bool TextureBatchDialog::showModal(RefArray< TextureAsset >& outAssets)
{
	if (ui::ConfigDialog::showModal() != ui::DrOk)
		return false;

	m_texturePropertyList->apply();

	for (int i = 0; i < m_textureList->count(); ++i)
		outAssets.push_back(checked_type_cast< TextureAsset* >(m_textureList->getData(i)));

	return true;
}

void TextureBatchDialog::addTexture()
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, i18n::Text(L"TEXTURE_BATCH_FILE_TITLE"), L"All files;*.*"))
		return;

	std::vector< Path > fileNames;
	if (fileDialog.showModal(fileNames) != ui::DrOk)
	{
		fileDialog.destroy();
		return;
	}
	fileDialog.destroy();

	std::wstring assetPath = m_editor->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");

	for (std::vector< Path >::iterator i = fileNames.begin(); i != fileNames.end(); ++i)
	{
		Path texturePath;
		if (!FileSystem::getInstance().getRelativePath(
			FileSystem::getInstance().getAbsolutePath(*i),
			FileSystem::getInstance().getAbsolutePath(assetPath),
			texturePath
		))
			texturePath = *i;

		Ref< TextureAsset > asset = new TextureAsset();
		asset->setFileName(texturePath);

		m_textureList->add(
			i->getFileName(),
			asset
		);
	}
}

void TextureBatchDialog::removeTexture()
{
	m_texturePropertyList->bind(0);
	m_texturePropertyList->update();

	for (int index = m_textureList->getSelected(); index >= 0; index = m_textureList->getSelected())
		m_textureList->remove(index);
}

void TextureBatchDialog::eventTextureListToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& cmd = event->getCommand();
	if (cmd == L"TextureBatch.Add")
		addTexture();
	else if (cmd == L"TextureBatch.Remove")
		removeTexture();
}

void TextureBatchDialog::eventTextureListSelect(ui::SelectionChangeEvent* event)
{
	Ref< TextureAsset > asset = checked_type_cast< TextureAsset* >(m_textureList->getSelectedData());
	m_texturePropertyList->apply();
	m_texturePropertyList->bind(asset);
	m_texturePropertyList->update();
}

	}
}
