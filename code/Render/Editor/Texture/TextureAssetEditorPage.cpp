#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "I18N/Text.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Editor/Texture/TextureAssetEditorPage.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/Image.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureAssetEditorPage", TextureAssetEditorPage, editor::IEditorPage)

TextureAssetEditorPage::TextureAssetEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool TextureAssetEditorPage::create(ui::Container* parent)
{
	m_asset = m_document->getObject< TextureAsset >(0);
	if (!m_asset)
		return false;

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	Ref< ui::ToolBar > toolBarEdit = new ui::ToolBar();
	toolBarEdit->create(container);
	//toolBarEdit->addImage(new ui::StyleBitmap(L"Script.RemoveBreakpoints"), 1);
	//toolBarEdit->addImage(new ui::StyleBitmap(L"Script.ToggleComments"), 1);
	toolBarEdit->addItem(new ui::ToolBarButton(L"R", ui::Command(L"Render.Texture.Editor.ToggleR")));
	toolBarEdit->addItem(new ui::ToolBarButton(L"G", ui::Command(L"Render.Texture.Editor.ToggleG")));
	toolBarEdit->addItem(new ui::ToolBarButton(L"B", ui::Command(L"Render.Texture.Editor.ToggleB")));
	toolBarEdit->addItem(new ui::ToolBarButton(L"A", ui::Command(L"Render.Texture.Editor.ToggleA")));
	//toolBarEdit->addEventHandler< ui::ToolBarButtonClickEvent >(this, &ScriptEditorPage::eventToolBarEditClick);

	Ref< ui::Container > imageContainer = new ui::Container();
	imageContainer->create(container, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0));

	m_imageTexture = new ui::Image();
	m_imageTexture->create(imageContainer, 0, ui::Image::WsTransparent | ui::WsDoubleBuffer);

	m_site->setPropertyObject(m_asset);

	updatePreview();
	return true;
}

void TextureAssetEditorPage::destroy()
{
	m_asset = nullptr;
}

bool TextureAssetEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool TextureAssetEditorPage::handleCommand(const ui::Command& command)
{
	return false;
}

void TextureAssetEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void TextureAssetEditorPage::updatePreview()
{
	//Ref< drawing::Image > textureThumb;

	std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, m_asset->getFileName());
	//int32_t size = ui::dpi96(128);

	Ref< drawing::Image > image = drawing::Image::load(fileName);
	// if (image)
	// {
	// 	StringOutputStream ss;
	// 	ss << L"Width " << image->getWidth() << Endl;
	// 	ss << L"Height " << image->getHeight() << Endl;
	// 	ss << L"Color bits " << image->getPixelFormat().getColorBits() << Endl;
	// 	ss << L"Alpha bits " << image->getPixelFormat().getAlphaBits() << Endl;

	// 	const auto imageInfo = image->getImageInfo();
	// 	if (imageInfo)
	// 	{
	// 		ss << L"Author " << imageInfo->getAuthor() << Endl;
	// 		ss << L"Copyright " << imageInfo->getCopyright() << Endl;
	// 		ss << L"Format " << imageInfo->getFormat() << Endl;
	// 		ss << L"Gamma " << imageInfo->getGamma() << Endl;
	// 	}

	// 	m_imageInfo->setText(ss.str());
	// }
	// else
	// 	m_imageInfo->setText(L"");

	// Ref< editor::IThumbnailGenerator > thumbnailGenerator = m_editor->getStoreObject< editor::IThumbnailGenerator >(L"ThumbnailGenerator");
	// if (!thumbnailGenerator)
	// 	return;

	const bool visibleAlpha = (m_asset->m_output.m_hasAlpha == true && m_asset->m_output.m_ignoreAlpha == false);
	const bool linearGamma = m_asset->m_output.m_linearGamma;

	// textureThumb = thumbnailGenerator->get(
	// 	fileName,
	// 	size,
	// 	size,
	// 	visibleAlpha ? editor::IThumbnailGenerator::AmWithAlpha : editor::IThumbnailGenerator::AmNoAlpha,
	// 	linearGamma ? editor::IThumbnailGenerator::GmLinear : editor::IThumbnailGenerator::GmSRGB
	// );
	if (image)
	{
		Ref< ui::Bitmap > textureBitmap = new ui::Bitmap(image);
		m_imageTexture->setImage(textureBitmap, false);
	}
	else
		m_imageTexture->setImage(nullptr, false);

	// textureThumb = thumbnailGenerator->get(
	// 	fileName,
	// 	size,
	// 	size,
	// 	editor::IThumbnailGenerator::AmNoAlpha,
	// 	linearGamma ? editor::IThumbnailGenerator::GmLinear : editor::IThumbnailGenerator::GmSRGB
	// );
	// if (textureThumb)
	// {
	// 	Ref< ui::Bitmap > textureBitmap = new ui::Bitmap(textureThumb);
	// 	m_imageTextureNoAlpha->setImage(textureBitmap, false);
	// }
	// else
	// 	m_imageTextureNoAlpha->setImage(nullptr, false);

	// textureThumb = thumbnailGenerator->get(
	// 	fileName,
	// 	size,
	// 	size,
	// 	editor::IThumbnailGenerator::AmAlphaOnly,
	// 	linearGamma ? editor::IThumbnailGenerator::GmLinear : editor::IThumbnailGenerator::GmSRGB
	// );
	// if (textureThumb)
	// {
	// 	Ref< ui::Bitmap > textureBitmap = new ui::Bitmap(textureThumb);
	// 	m_imageTextureAlphaOnly->setImage(textureBitmap, false);
	// }
	// else
	// 	m_imageTextureAlphaOnly->setImage(nullptr, false);
}

// void TextureAssetEditorPage::eventPropertyCommand(ui::PropertyCommandEvent* event)
// {
// 	const ui::Command& cmd = event->getCommand();
// 	if (cmd == L"Property.Add")
// 	{
// 		ui::ArrayPropertyItem* arrayItem = dynamic_type_cast< ui::ArrayPropertyItem* >(event->getItem());
// 		if (arrayItem)
// 		{
// 			if (arrayItem->getElementType())
// 			{
// 				const TypeInfo* objectType = m_editor->browseType(makeTypeInfoSet(*arrayItem->getElementType()), false, true);
// 				if (objectType)
// 				{
// 					Ref< ISerializable > object = dynamic_type_cast< ISerializable* >(objectType->createInstance());
// 					if (object)
// 					{
// 						m_propertyList->addObject(arrayItem, object);
// 						m_propertyList->apply();
// 						m_propertyList->refresh();
// 					}
// 				}
// 			}
// 			else	// Non-complex array; just apply and refresh.
// 			{
// 				m_propertyList->apply();
// 				m_propertyList->refresh();
// 			}
// 		}
// 	}
// 	else if (cmd == L"Property.Remove")
// 	{
// 		ui::PropertyItem* removeItem = event->getItem();
// 		ui::PropertyItem* parentItem = removeItem->getParentItem();
// 		if (parentItem)
// 		{
// 			m_propertyList->removePropertyItem(parentItem, removeItem);
// 			m_propertyList->apply();
// 		}
// 	}
// 	else if (cmd == L"Property.Browse")
// 	{
// 		ui::BrowsePropertyItem* browseItem = dynamic_type_cast< ui::BrowsePropertyItem* >(event->getItem());
// 		if (browseItem)
// 		{
// 			if (browseItem->getValue().isNull())
// 			{
// 				Ref< db::Instance > instance;
// 				if (browseItem->getFilterType())
// 				{
// 					const TypeInfo* filterType = browseItem->getFilterType();
// 					T_ASSERT(filterType);

// 					instance = m_editor->browseInstance(*filterType);
// 				}
// 				else
// 					instance = m_editor->browseInstance();

// 				if (instance)
// 				{
// 					browseItem->setValue(instance->getGuid());
// 					m_propertyList->apply();
// 				}
// 			}
// 			else
// 			{
// 				browseItem->setValue(Guid());
// 				m_propertyList->apply();
// 			}
// 		}

// 		ui::FilePropertyItem* fileItem = dynamic_type_cast< ui::FilePropertyItem* >(event->getItem());
// 		if (fileItem)
// 		{
// 			ui::FileDialog fileDialog;
// 			if (!fileDialog.create(m_propertyList, type_name(this), i18n::Text(L"EDITOR_BROWSE_FILE"), L"All files (*.*);*.*"))
// 				return;

// 			// Convert path to absolute path.
// 			Path assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
// 			Path path = FileSystem::getInstance().getAbsolutePath(assetPath, fileItem->getPath());

// 			if (fileDialog.showModal(path) == ui::DrOk)
// 			{
// 				// Convert path to be relative to asset path.
// 				Path relativePath;
// 				if (FileSystem::getInstance().getRelativePath(path, FileSystem::getInstance().getAbsolutePath(assetPath), relativePath))
// 					path = relativePath;

// 				fileItem->setPath(path);
// 				m_propertyList->apply();
// 			}

// 			fileDialog.destroy();
// 		}

// 		ui::ObjectPropertyItem* objectItem = dynamic_type_cast< ui::ObjectPropertyItem* >(event->getItem());
// 		if (objectItem)
// 		{
// 			const TypeInfo* objectType = objectItem->getObjectType();
// 			if (!objectType)
// 				objectType = &type_of< ISerializable >();

// 			if (!objectItem->getObject())
// 			{
// 				objectType = m_editor->browseType(makeTypeInfoSet(*objectType), false, true);
// 				if (objectType)
// 				{
// 					Ref< ISerializable > object = dynamic_type_cast< ISerializable* >(objectType->createInstance());
// 					if (object)
// 					{
// 						objectItem->setObject(object);

// 						m_propertyList->refresh(objectItem, object);
// 						m_propertyList->apply();
// 					}
// 				}
// 			}
// 			else
// 			{
// 				if (ui::ArrayPropertyItem* parentArrayItem = dynamic_type_cast< ui::ArrayPropertyItem* >(objectItem->getParentItem()))
// 					m_propertyList->removePropertyItem(parentArrayItem, objectItem);
// 				else
// 					objectItem->setObject(0);

// 				m_propertyList->refresh(objectItem, 0);
// 				m_propertyList->apply();
// 			}
// 		}
// 	}
// 	else if (cmd == L"Property.Edit")
// 	{
// 		ui::BrowsePropertyItem* browseItem = dynamic_type_cast< ui::BrowsePropertyItem* >(event->getItem());
// 		if (browseItem)
// 		{
// 			Guid instanceGuid = browseItem->getValue();
// 			if (instanceGuid.isNull() || !instanceGuid.isValid())
// 				return;

// 			Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(instanceGuid);
// 			if (!instance)
// 				return;

// 			m_editor->openEditor(instance);
// 		}

// /*
// 		ui::TextPropertyItem* textItem = dynamic_type_cast< ui::TextPropertyItem* >(event->getItem());
// 		if (textItem)
// 		{
// 			TextEditorDialog textEditorDialog;
// 			textEditorDialog.create(m_propertyList, textItem->getValue());
// 			if (textEditorDialog.showModal() == ui::DrOk)
// 			{
// 				textItem->setValue(textEditorDialog.getText());
// 				m_propertyList->apply();
// 			}
// 			textEditorDialog.destroy();
// 		}

// 		ui::ColorPropertyItem* colorItem = dynamic_type_cast< ui::ColorPropertyItem* >(event->getItem());
// 		if (colorItem)
// 		{
// 			ui::ColorDialog colorDialog;
// 			colorDialog.create(m_propertyList, i18n::Text(L"COLOR_DIALOG_TEXT"), ui::ColorDialog::WsDefaultFixed | ui::ColorDialog::WsAlpha, colorItem->getValue());
// 			if (colorDialog.showModal() == ui::DrOk)
// 			{
// 				colorItem->setValue(colorDialog.getColor());
// 				m_propertyList->apply();
// 			}
// 			colorDialog.destroy();
// 		}
// */
// 	}
// 	m_propertyList->update();
// 	updatePreview();
// }

// void TextureAssetEditorPage::eventPropertyContentChangeEvent(ui::PropertyContentChangeEvent* event)
// {
// 	m_propertyList->apply();
// 	updatePreview();
// }

// bool TextureAssetEditorPage::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
// {
// 	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(guid);
// 	if (!instance)
// 		return false;

// 	resolved = instance->getPath();
// 	return true;
// }

	}
}
