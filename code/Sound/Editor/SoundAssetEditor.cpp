/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Sound/Sound.h"
#include "Sound/StreamAudioBuffer.h"
#include "Sound/Decoders/FlacStreamDecoder.h"
#include "Sound/Decoders/Mp3StreamDecoder.h"
#include "Sound/Decoders/OggStreamDecoder.h"
#include "Sound/Decoders/TssStreamDecoder.h"
#include "Sound/Decoders/WavStreamDecoder.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/SoundAssetEditor.h"
#include "Sound/Player/ISoundHandle.h"
#include "Sound/Player/ISoundPlayer.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/FileDialog.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/PropertyList/ArrayPropertyItem.h"
#include "Ui/PropertyList/BrowsePropertyItem.h"
#include "Ui/PropertyList/FilePropertyItem.h"
#include "Ui/PropertyList/ObjectPropertyItem.h"
#include "Ui/PropertyList/PropertyCommandEvent.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundAssetEditor", SoundAssetEditor, editor::IObjectEditor)

SoundAssetEditor::SoundAssetEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool SoundAssetEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	m_asset = mandatory_non_null_type_cast< SoundAsset* >(object);

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut));

	m_toolBar = new ui::ToolBar();
	m_toolBar->create(container);
	m_toolBar->addImage(new ui::StyleBitmap(L"Sound.Play"));
	m_toolBar->addItem(new ui::ToolBarButton(L"Play", 0, ui::Command(L"Sound.Play")));
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &SoundAssetEditor::eventToolBarClick);

	m_propertyList = new ui::AutoPropertyList();
	m_propertyList->create(container, ui::WsDoubleBuffer | ui::AutoPropertyList::WsColumnHeader, this);
	m_propertyList->addEventHandler< ui::PropertyCommandEvent >(this, &SoundAssetEditor::eventPropertyCommand);
	m_propertyList->setSeparator(200_ut);
	m_propertyList->setColumnName(0, i18n::Text(L"PROPERTY_COLUMN_NAME"));
	m_propertyList->setColumnName(1, i18n::Text(L"PROPERTY_COLUMN_VALUE"));
	m_propertyList->bind(m_asset);

	// Get audio player for preview.
	m_soundPlayer = m_editor->getObjectStore()->get< ISoundPlayer >();

	return true;
}

void SoundAssetEditor::destroy()
{
	if (m_soundHandle)
		m_soundHandle->stop();

	m_soundPlayer = nullptr;

	safeDestroy(m_propertyList);
	safeDestroy(m_toolBar);

	m_instance = nullptr;
	m_asset = nullptr;
}

void SoundAssetEditor::apply()
{
	m_propertyList->apply();
	m_instance->setObject(m_asset);
}

bool SoundAssetEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void SoundAssetEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

ui::Size SoundAssetEditor::getPreferredSize() const
{
	return ui::Size(
		500,
		400
	);
}

void SoundAssetEditor::eventToolBarClick(ui::ToolBarButtonClickEvent* event)
{
	if (!m_soundPlayer)
	{
		log::error << L"Failed to preview sound asset; no audio player." << Endl;
		return;
	}

	const std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	const Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, m_asset->getFileName());

	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!file)
	{
		log::error << L"Failed to preview sound asset; unable to open file." << Endl;
		return;
	}

	Ref< IStreamDecoder > decoder;
	if (compareIgnoreCase(fileName.getExtension(), L"wav") == 0)
		decoder = new sound::WavStreamDecoder();
	else if (compareIgnoreCase(fileName.getExtension(), L"flac") == 0)
		decoder = new sound::FlacStreamDecoder();
	else if (compareIgnoreCase(fileName.getExtension(), L"mp3") == 0)
		decoder = new sound::Mp3StreamDecoder();
	else if (compareIgnoreCase(fileName.getExtension(), L"ogg") == 0)
		decoder = new sound::OggStreamDecoder();
	else if (compareIgnoreCase(fileName.getExtension(), L"tss") == 0)
		decoder = new sound::TssStreamDecoder();
	else
	{
		log::error << L"Failed to preview sound asset; unable to determine decoder from extension." << Endl;
		return;
	}

	if (!decoder->create(file))
	{
		log::error << L"Failed to preview sound asset; unable to create decoder" << Endl;
		return;
	}

	Ref< StreamAudioBuffer > buffer = new StreamAudioBuffer();
	if (!buffer->create(decoder))
	{
		log::error << L"Failed to preview sound asset; unable to create stream buffer" << Endl;
		return;
	}

	if (m_soundHandle)
		m_soundHandle->stop();

	m_soundHandle = m_soundPlayer->play(new Sound(buffer, 0, m_asset->getGain(), 0.0f), 0);
}

void SoundAssetEditor::eventPropertyCommand(ui::PropertyCommandEvent* event)
{
	const ui::Command& cmd = event->getCommand();
	if (cmd == L"Property.Add")
	{
		ui::ArrayPropertyItem* arrayItem = dynamic_type_cast< ui::ArrayPropertyItem* >(event->getItem());
		if (arrayItem)
		{
			if (arrayItem->getElementType())
			{
				const TypeInfo* objectType = m_editor->browseType(makeTypeInfoSet(*arrayItem->getElementType()), false, true);
				if (objectType)
				{
					Ref< ISerializable > object = dynamic_type_cast< ISerializable* >(objectType->createInstance());
					if (object)
					{
						m_propertyList->addObject(arrayItem, object);
						m_propertyList->apply();
						m_propertyList->refresh();
					}
				}
			}
			else	// Non-complex array; just apply and refresh.
			{
				m_propertyList->apply();
				m_propertyList->refresh();
			}
		}
	}
	else if (cmd == L"Property.Remove")
	{
		ui::PropertyItem* removeItem = event->getItem();
		ui::PropertyItem* parentItem = removeItem->getParentItem();
		if (parentItem)
		{
			m_propertyList->removePropertyItem(parentItem, removeItem);
			m_propertyList->apply();
		}
	}
	else if (cmd == L"Property.Browse")
	{
		ui::BrowsePropertyItem* browseItem = dynamic_type_cast< ui::BrowsePropertyItem* >(event->getItem());
		if (browseItem)
		{
			if (browseItem->getValue().isNull())
			{
				Ref< db::Instance > instance;
				if (browseItem->getFilterType())
				{
					const TypeInfo* filterType = browseItem->getFilterType();
					T_ASSERT(filterType);

					instance = m_editor->browseInstance(*filterType);
				}
				else
					instance = m_editor->browseInstance();

				if (instance)
				{
					browseItem->setValue(instance->getGuid());
					m_propertyList->apply();
				}
			}
			else
			{
				browseItem->setValue(Guid());
				m_propertyList->apply();
			}
		}

		ui::FilePropertyItem* fileItem = dynamic_type_cast< ui::FilePropertyItem* >(event->getItem());
		if (fileItem)
		{
			ui::FileDialog fileDialog;
			if (!fileDialog.create(m_propertyList, type_name(this), i18n::Text(L"EDITOR_BROWSE_FILE"), L"All files (*.*);*.*"))
				return;

			Path path = fileItem->getPath();
			if (fileDialog.showModal(path) == ui::DialogResult::Ok)
			{
				fileItem->setPath(path);
				m_propertyList->apply();
			}

			fileDialog.destroy();
		}

		ui::ObjectPropertyItem* objectItem = dynamic_type_cast< ui::ObjectPropertyItem* >(event->getItem());
		if (objectItem)
		{
			const TypeInfo* objectType = objectItem->getObjectType();
			if (!objectType)
				objectType = &type_of< ISerializable >();

			if (!objectItem->getObject())
			{
				objectType = m_editor->browseType(makeTypeInfoSet(*objectType), false, true);
				if (objectType)
				{
					Ref< ISerializable > object = dynamic_type_cast< ISerializable* >(objectType->createInstance());
					if (object)
					{
						objectItem->setObject(object);

						m_propertyList->refresh(objectItem, object);
						m_propertyList->apply();
					}
				}
			}
			else
			{
				if (ui::ArrayPropertyItem* parentArrayItem = dynamic_type_cast< ui::ArrayPropertyItem* >(objectItem->getParentItem()))
					m_propertyList->removePropertyItem(parentArrayItem, objectItem);
				else
					objectItem->setObject(nullptr);

				m_propertyList->refresh(objectItem, nullptr);
				m_propertyList->apply();
			}
		}
	}
	else if (cmd == L"Property.Edit")
	{
		ui::BrowsePropertyItem* browseItem = dynamic_type_cast< ui::BrowsePropertyItem* >(event->getItem());
		if (browseItem)
		{
			Guid instanceGuid = browseItem->getValue();
			if (instanceGuid.isNull() || !instanceGuid.isValid())
				return;

			Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(instanceGuid);
			if (!instance)
				return;

			m_editor->openEditor(instance);
		}

/*
		ui::TextPropertyItem* textItem = dynamic_type_cast< ui::TextPropertyItem* >(event->getItem());
		if (textItem)
		{
			TextEditorDialog textEditorDialog;
			textEditorDialog.create(m_propertyList, textItem->getValue());
			if (textEditorDialog.showModal() == ui::DialogResult::Ok)
			{
				textItem->setValue(textEditorDialog.getText());
				m_propertyList->apply();
			}
			textEditorDialog.destroy();
		}

		ui::ColorPropertyItem* colorItem = dynamic_type_cast< ui::ColorPropertyItem* >(event->getItem());
		if (colorItem)
		{
			ui::ColorDialog colorDialog;
			colorDialog.create(m_propertyList, i18n::Text(L"COLOR_DIALOG_TEXT"), ui::ColorDialog::WsDefaultFixed | ui::ColorDialog::WsAlpha, colorItem->getValue());
			if (colorDialog.showModal() == ui::DialogResult::Ok)
			{
				colorItem->setValue(colorDialog.getColor());
				m_propertyList->apply();
			}
			colorDialog.destroy();
		}
*/
	}
	m_propertyList->update();
}

bool SoundAssetEditor::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
{
	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(guid);
	if (!instance)
		return false;

	resolved = instance->getPath();
	return true;
}

}
