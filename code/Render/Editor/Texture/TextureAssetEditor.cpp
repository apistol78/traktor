#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IThumbnailGenerator.h"
#include "I18N/Text.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Editor/Texture/TextureAssetEditor.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/FileDialog.h"
#include "Ui/FloodLayout.h"
#include "Ui/Image.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/PropertyList/ArrayPropertyItem.h"
#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/FilePropertyItem.h"
#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyCommandEvent.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureAssetEditor", TextureAssetEditor, editor::IObjectEditor)

TextureAssetEditor::TextureAssetEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool TextureAssetEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	m_asset = checked_type_cast< TextureAsset* >(object);

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"*,100%", L"100%", 0, 0));

	Ref< ui::Container > imageContainer = new ui::Container();
	imageContainer->create(container, ui::WsNone, new ui::TableLayout(toString(ui::scaleBySystemDPI(128)), L"*,*,*,100%", 8, 8));

	m_imageTextureWithAlpha = new ui::Image();
	m_imageTextureWithAlpha->create(imageContainer, 0, ui::Image::WsTransparent | ui::WsDoubleBuffer);

	m_imageTextureNoAlpha = new ui::Image();
	m_imageTextureNoAlpha->create(imageContainer, 0, ui::Image::WsTransparent | ui::WsDoubleBuffer);

	m_imageTextureAlphaOnly = new ui::Image();
	m_imageTextureAlphaOnly->create(imageContainer, 0, ui::Image::WsTransparent | ui::WsDoubleBuffer);

	m_propertyList = new ui::custom::AutoPropertyList();
	m_propertyList->create(container, ui::WsDoubleBuffer | ui::custom::AutoPropertyList::WsColumnHeader, this);
	m_propertyList->addEventHandler< ui::custom::PropertyCommandEvent >(this, &TextureAssetEditor::eventPropertyCommand);
	m_propertyList->setSeparator(ui::scaleBySystemDPI(200));
	m_propertyList->setColumnName(0, i18n::Text(L"PROPERTY_COLUMN_NAME"));
	m_propertyList->setColumnName(1, i18n::Text(L"PROPERTY_COLUMN_VALUE"));
	m_propertyList->bind(m_asset);

	updatePreview();
	return true;
}

void TextureAssetEditor::destroy()
{
	safeDestroy(m_propertyList);

	m_instance = 0;
	m_asset = 0;
}

void TextureAssetEditor::apply()
{
	m_propertyList->apply();
	m_instance->setObject(m_asset);
}

bool TextureAssetEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void TextureAssetEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

ui::Size TextureAssetEditor::getPreferredSize() const
{
	return ui::Size(
		ui::scaleBySystemDPI(850),
		ui::scaleBySystemDPI(550)
	);
}

void TextureAssetEditor::updatePreview()
{
	Ref< drawing::Image > textureThumb;

	Ref< editor::IThumbnailGenerator > thumbnailGenerator = m_editor->getStoreObject< editor::IThumbnailGenerator >(L"ThumbnailGenerator");
	if (!thumbnailGenerator)
		return;

	std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, m_asset->getFileName());
	int32_t size = ui::scaleBySystemDPI(128);

	bool visibleAlpha = (m_asset->m_output.m_hasAlpha == true && m_asset->m_output.m_ignoreAlpha == false);
	textureThumb = thumbnailGenerator->get(
		fileName,
		size,
		size,
		visibleAlpha ? editor::IThumbnailGenerator::AmWithAlpha : editor::IThumbnailGenerator::AmNoAlpha
	);
	if (textureThumb)
	{
		Ref< ui::Bitmap > textureBitmap = new ui::Bitmap(textureThumb);
		m_imageTextureWithAlpha->setImage(textureBitmap, false);
	}
	else
		m_imageTextureWithAlpha->setImage(0, false);

	textureThumb = thumbnailGenerator->get(
		fileName,
		size,
		size,
		editor::IThumbnailGenerator::AmNoAlpha
	);
	if (textureThumb)
	{
		Ref< ui::Bitmap > textureBitmap = new ui::Bitmap(textureThumb);
		m_imageTextureNoAlpha->setImage(textureBitmap, false);
	}
	else
		m_imageTextureNoAlpha->setImage(0, false);

	textureThumb = thumbnailGenerator->get(
		fileName,
		size,
		size,
		editor::IThumbnailGenerator::AmAlphaOnly
	);
	if (textureThumb)
	{
		Ref< ui::Bitmap > textureBitmap = new ui::Bitmap(textureThumb);
		m_imageTextureAlphaOnly->setImage(textureBitmap, false);
	}
	else
		m_imageTextureAlphaOnly->setImage(0, false);
}

void TextureAssetEditor::eventPropertyCommand(ui::custom::PropertyCommandEvent* event)
{
	const ui::Command& cmd = event->getCommand();
	if (cmd == L"Property.Add")
	{
		ui::custom::ArrayPropertyItem* arrayItem = dynamic_type_cast< ui::custom::ArrayPropertyItem* >(event->getItem());
		if (arrayItem)
		{
			if (arrayItem->getElementType())
			{
				const TypeInfo* objectType = m_editor->browseType(makeTypeInfoSet(*arrayItem->getElementType()));
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
		ui::custom::PropertyItem* removeItem = event->getItem();
		ui::custom::PropertyItem* parentItem = removeItem->getParentItem();
		if (parentItem)
		{
			m_propertyList->removePropertyItem(parentItem, removeItem);
			m_propertyList->apply();
		}
	}
	else if (cmd == L"Property.Browse")
	{
		ui::custom::BrowsePropertyItem* browseItem = dynamic_type_cast< ui::custom::BrowsePropertyItem* >(event->getItem());
		if (browseItem)
		{
			if (browseItem->getValue().isNull())
			{
				Ref< db::Instance > instance;
				if (browseItem->getFilterType())
				{
					const TypeInfo* filterType = browseItem->getFilterType();
					T_ASSERT (filterType);

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

		ui::custom::FilePropertyItem* fileItem = dynamic_type_cast< ui::custom::FilePropertyItem* >(event->getItem());
		if (fileItem)
		{
			ui::FileDialog fileDialog;
			if (!fileDialog.create(m_propertyList, i18n::Text(L"EDITOR_BROWSE_FILE"), L"All files (*.*);*.*"))
				return;

			Path path = fileItem->getPath();
			if (fileDialog.showModal(path) == ui::DrOk)
			{
				fileItem->setPath(path);
				m_propertyList->apply();
			}

			fileDialog.destroy();
		}

		ui::custom::ObjectPropertyItem* objectItem = dynamic_type_cast< ui::custom::ObjectPropertyItem* >(event->getItem());
		if (objectItem)
		{
			const TypeInfo* objectType = objectItem->getObjectType();
			if (!objectType)
				objectType = &type_of< ISerializable >();

			if (!objectItem->getObject())
			{
				objectType = m_editor->browseType(makeTypeInfoSet(*objectType));
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
				if (ui::custom::ArrayPropertyItem* parentArrayItem = dynamic_type_cast< ui::custom::ArrayPropertyItem* >(objectItem->getParentItem()))
					m_propertyList->removePropertyItem(parentArrayItem, objectItem);
				else
					objectItem->setObject(0);

				m_propertyList->refresh(objectItem, 0);
				m_propertyList->apply();
			}
		}
	}
	else if (cmd == L"Property.Edit")
	{
		ui::custom::BrowsePropertyItem* browseItem = dynamic_type_cast< ui::custom::BrowsePropertyItem* >(event->getItem());
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
		ui::custom::TextPropertyItem* textItem = dynamic_type_cast< ui::custom::TextPropertyItem* >(event->getItem());
		if (textItem)
		{
			TextEditorDialog textEditorDialog;
			textEditorDialog.create(m_propertyList, textItem->getValue());
			if (textEditorDialog.showModal() == ui::DrOk)
			{
				textItem->setValue(textEditorDialog.getText());
				m_propertyList->apply();
			}
			textEditorDialog.destroy();
		}

		ui::custom::ColorPropertyItem* colorItem = dynamic_type_cast< ui::custom::ColorPropertyItem* >(event->getItem());
		if (colorItem)
		{
			ui::custom::ColorDialog colorDialog;
			colorDialog.create(m_propertyList, i18n::Text(L"COLOR_DIALOG_TEXT"), ui::custom::ColorDialog::WsDefaultFixed | ui::custom::ColorDialog::WsAlpha, colorItem->getValue());
			if (colorDialog.showModal() == ui::DrOk)
			{
				colorItem->setValue(colorDialog.getColor());
				m_propertyList->apply();
			}
			colorDialog.destroy();
		}
*/
	}
	m_propertyList->update();
	updatePreview();
}

bool TextureAssetEditor::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
{
	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(guid);
	if (!instance)
		return false;

	resolved = instance->getPath();
	return true;
}

	}
}
