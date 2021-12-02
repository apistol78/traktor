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
#include "Editor/PropertiesView.h"
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

	Ref< ui::ToolBar > toolBar = new ui::ToolBar();
	toolBar->create(container);
	toolBar->addItem(new ui::ToolBarButton(L"R", ui::Command(L"Render.Texture.Editor.ToggleR"), ui::ToolBarButton::BsDefaultToggled));
	toolBar->addItem(new ui::ToolBarButton(L"G", ui::Command(L"Render.Texture.Editor.ToggleG"), ui::ToolBarButton::BsDefaultToggled));
	toolBar->addItem(new ui::ToolBarButton(L"B", ui::Command(L"Render.Texture.Editor.ToggleB"), ui::ToolBarButton::BsDefaultToggled));
	toolBar->addItem(new ui::ToolBarButton(L"A", ui::Command(L"Render.Texture.Editor.ToggleA"), ui::ToolBarButton::BsDefaultToggled));

	Ref< ui::Container > imageContainer = new ui::Container();
	imageContainer->create(container, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0));

	m_imageTexture = new ui::Image();
	m_imageTexture->create(imageContainer, 0, ui::Image::WsTransparent | ui::WsDoubleBuffer);

	// Create properties view.
	m_propertiesView = m_site->createPropertiesView(parent);
	m_propertiesView->setPropertyObject(m_asset);
	m_propertiesView->addEventHandler< ui::ContentChangeEvent >(this, &TextureAssetEditorPage::eventPropertiesChanged);
	m_site->createAdditionalPanel(m_propertiesView, ui::dpi96(400), false);

	updatePreview();
	return true;
}

void TextureAssetEditorPage::destroy()
{
	if (m_propertiesView)
		m_site->destroyAdditionalPanel(m_propertiesView);

	safeDestroy(m_propertiesView);

	m_asset = nullptr;
}

bool TextureAssetEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool TextureAssetEditorPage::handleCommand(const ui::Command& command)
{
	if (m_propertiesView->handleCommand(command))
		return true;
	
	return false;
}

void TextureAssetEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void TextureAssetEditorPage::updatePreview()
{
	std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, m_asset->getFileName());

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

void TextureAssetEditorPage::eventPropertiesChanged(ui::ContentChangeEvent* event)
{
	updatePreview();
}

	}
}
