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
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/SoundBatchDialog.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/FileDialog.h"
#include "Ui/Splitter.h"
#include "Ui/ListBox/ListBox.h"
#include "Ui/PropertyList/AutoPropertyList.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundBatchDialog", SoundBatchDialog, ui::ConfigDialog)

SoundBatchDialog::SoundBatchDialog(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool SoundBatchDialog::create(ui::Widget* parent)
{
	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"SOUND_BATCH_DIALOG_TITLE"),
		ui::dpi96(900),
		ui::dpi96(500),
		ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
		new ui::FloodLayout()
	))
		return false;

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(this, true, ui::dpi96(200));

	Ref< ui::Container > soundListContainer = new ui::Container();
	soundListContainer->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	Ref< ui::ToolBar > soundListTools = new ui::ToolBar();
	if (!soundListTools->create(soundListContainer))
		return false;

	soundListTools->addImage(new ui::StyleBitmap(L"Sound.PlusMinus"), 2);
	soundListTools->addItem(new ui::ToolBarButton(i18n::Text(L"SOUND_BATCH_ADD"), 0, ui::Command(L"SoundBatch.Add")));
	soundListTools->addItem(new ui::ToolBarButton(i18n::Text(L"SOUND_BATCH_REMOVE"), 1, ui::Command(L"SoundBatch.Remove")));
	soundListTools->addEventHandler< ui::ToolBarButtonClickEvent >(this, &SoundBatchDialog::eventSoundListToolClick);

	m_soundList = new ui::ListBox();
	m_soundList->create(soundListContainer, ui::ListBox::WsExtended);
	m_soundList->addEventHandler< ui::SelectionChangeEvent >(this, &SoundBatchDialog::eventSoundListSelect);

	m_soundPropertyList = new ui::AutoPropertyList();
	m_soundPropertyList->create(splitter, ui::WsDoubleBuffer | ui::AutoPropertyList::WsColumnHeader);
	m_soundPropertyList->setSeparator(ui::dpi96(200));
	m_soundPropertyList->setColumnName(0, i18n::Text(L"PROPERTY_COLUMN_NAME"));
	m_soundPropertyList->setColumnName(1, i18n::Text(L"PROPERTY_COLUMN_VALUE"));

	return true;
}

void SoundBatchDialog::destroy()
{
	ui::ConfigDialog::destroy();
}

bool SoundBatchDialog::showModal(RefArray< SoundAsset >& outAssets)
{
	if (ui::ConfigDialog::showModal() != ui::DialogResult::Ok)
		return false;

	m_soundPropertyList->apply();

	for (int i = 0; i < m_soundList->count(); ++i)
		outAssets.push_back(checked_type_cast< SoundAsset* >(m_soundList->getData(i)));

	return true;
}

void SoundBatchDialog::addSound()
{
	std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), i18n::Text(L"SOUND_BATCH_FILE_TITLE"), L"All files;*.*", assetPath))
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
		Path soundPath;
		if (!FileSystem::getInstance().getRelativePath(
			FileSystem::getInstance().getAbsolutePath(fileName),
			FileSystem::getInstance().getAbsolutePath(assetPath),
			soundPath
		))
			soundPath = fileName;

		Ref< SoundAsset > asset = new SoundAsset();
		asset->setFileName(soundPath);

		m_soundList->add(
			fileName.getFileName(),
			asset
		);
	}
}

void SoundBatchDialog::removeSound()
{
	m_soundPropertyList->bind(nullptr);
	m_soundPropertyList->update();

	for (int index = m_soundList->getSelected(); index >= 0; index = m_soundList->getSelected())
		m_soundList->remove(index);
}

void SoundBatchDialog::eventSoundListToolClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& cmd = event->getCommand();
	if (cmd == L"SoundBatch.Add")
		addSound();
	else if (cmd == L"SoundBatch.Remove")
		removeSound();
}

void SoundBatchDialog::eventSoundListSelect(ui::SelectionChangeEvent* event)
{
	Ref< SoundAsset > asset = checked_type_cast< SoundAsset* >(m_soundList->getSelectedData());
	m_soundPropertyList->apply();
	m_soundPropertyList->bind(asset);
	m_soundPropertyList->update();
}

	}
}
