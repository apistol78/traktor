/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Editor/Texture/TextureBatchDialog.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/FileDialog.h"
#include "Ui/ListBox/ListBox.h"
#include "Ui/Splitter.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/PropertyList/AutoPropertyList.h"

namespace traktor::render
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
		900_ut,
		500_ut,
		ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
		new ui::FloodLayout()
	))
		return false;

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(this, true, 200_ut);

	Ref< ui::Container > textureListContainer = new ui::Container();
	textureListContainer->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut));

	Ref< ui::ToolBar > textureListTools = new ui::ToolBar();
	if (!textureListTools->create(textureListContainer))
		return false;

	textureListTools->addImage(new ui::StyleBitmap(L"Texture.PlusMinus"), 2);
	textureListTools->addItem(new ui::ToolBarButton(i18n::Text(L"TEXTURE_BATCH_ADD"), 0, ui::Command(L"TextureBatch.Add")));
	textureListTools->addItem(new ui::ToolBarButton(i18n::Text(L"TEXTURE_BATCH_REMOVE"), 1, ui::Command(L"TextureBatch.Remove")));
	textureListTools->addEventHandler< ui::ToolBarButtonClickEvent >(this, &TextureBatchDialog::eventTextureListToolClick);

	m_textureList = new ui::ListBox();
	m_textureList->create(textureListContainer, ui::ListBox::WsExtended);
	m_textureList->addEventHandler< ui::SelectionChangeEvent >(this, &TextureBatchDialog::eventTextureListSelect);

	m_texturePropertyList = new ui::AutoPropertyList();
	m_texturePropertyList->create(splitter, ui::WsAccelerated | ui::AutoPropertyList::WsColumnHeader);
	m_texturePropertyList->setSeparator(200_ut);
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
	if (ui::ConfigDialog::showModal() != ui::DialogResult::Ok)
		return false;

	m_texturePropertyList->apply();

	for (int i = 0; i < m_textureList->count(); ++i)
		outAssets.push_back(checked_type_cast< TextureAsset* >(m_textureList->getData(i)));

	return true;
}

void TextureBatchDialog::addTexture()
{
	std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), i18n::Text(L"TEXTURE_BATCH_FILE_TITLE"), L"All files;*.*", assetPath))
		return;

	std::vector< Path > fileNames;
	if (fileDialog.showModal(fileNames) != ui::DialogResult::Ok)
	{
		fileDialog.destroy();
		return;
	}
	fileDialog.destroy();

	for (const auto& fileName : fileNames)
	{
		Path texturePath;
		if (!FileSystem::getInstance().getRelativePath(
			FileSystem::getInstance().getAbsolutePath(fileName),
			FileSystem::getInstance().getAbsolutePath(assetPath),
			texturePath
		))
			texturePath = fileName;

		Ref< TextureAsset > asset = new TextureAsset();
		asset->setFileName(texturePath);

		m_textureList->add(
			fileName.getFileName(),
			asset
		);
	}
}

void TextureBatchDialog::removeTexture()
{
	m_texturePropertyList->bind(nullptr);
	m_texturePropertyList->update();

	for (int index = m_textureList->getSelected(); index >= 0; index = m_textureList->getSelected())
		m_textureList->remove(index);
}

void TextureBatchDialog::eventTextureListToolClick(ui::ToolBarButtonClickEvent* event)
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
