#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Render/Editor/Texture/TextureAsset.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.TextureAsset", 5, TextureAsset, TextureAssetBase)

Ref< drawing::Image > TextureAsset::load(const std::wstring& assetPath) const
{
	Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, getFileName());
	Ref< drawing::Image > image = drawing::Image::load(fileName);
	if (image)
		return image;
	else
	{
		log::error << L"Unable to read source texture image \"" << fileName.getPathName() << L"\"" << Endl;
		return 0;
	}
		
}

	}
}
