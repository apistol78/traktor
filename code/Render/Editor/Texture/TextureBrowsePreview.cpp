#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IThumbnailGenerator.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Editor/Texture/TextureBrowsePreview.h"
#include "Ui/Bitmap.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureBrowsePreview", 0, TextureBrowsePreview, editor::IBrowsePreview)

TypeInfoSet TextureBrowsePreview::getPreviewTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TextureAsset >());
	return typeSet;
}

Ref< ui::Bitmap > TextureBrowsePreview::generate(const editor::IEditor* editor, db::Instance* instance) const
{
	Ref< const TextureAsset > textureAsset = instance->getObject< TextureAsset >();
	if (!textureAsset)
		return 0;

	Ref< editor::IThumbnailGenerator > thumbnailGenerator = editor->getStoreObject< editor::IThumbnailGenerator >(L"ThumbnailGenerator");
	if (!thumbnailGenerator)
		return 0;

	std::wstring assetPath = editor->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, textureAsset->getFileName());

	bool visibleAlpha = (textureAsset->m_output.m_hasAlpha == true && textureAsset->m_output.m_ignoreAlpha == false);
	Ref< drawing::Image > textureThumb = thumbnailGenerator->get(
		fileName,
		64,
		64,
		visibleAlpha ? editor::IThumbnailGenerator::AmWithAlpha : editor::IThumbnailGenerator::AmNoAlpha
	);

	return textureThumb ? new ui::Bitmap(textureThumb) : 0;
}

	}
}
