/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IThumbnailGenerator.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Editor/Texture/TextureBrowsePreview.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureBrowsePreview", 0, TextureBrowsePreview, editor::IBrowsePreview)

TypeInfoSet TextureBrowsePreview::getPreviewTypes() const
{
	return makeTypeInfoSet< TextureAsset >();
}

Ref< ui::IBitmap > TextureBrowsePreview::generate(editor::IEditor* editor, db::Instance* instance) const
{
	Ref< const TextureAsset > textureAsset = instance->getObject< TextureAsset >();
	if (!textureAsset)
		return nullptr;

	Ref< editor::IThumbnailGenerator > thumbnailGenerator = editor->getObjectStore()->get< editor::IThumbnailGenerator >();
	if (!thumbnailGenerator)
		return nullptr;

	const std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	const Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, textureAsset->getFileName());

	const bool visibleAlpha = (textureAsset->m_output.m_hasAlpha == true && textureAsset->m_output.m_ignoreAlpha == false);
	const bool linearGamma = textureAsset->m_output.m_assumeLinearGamma;
	Ref< drawing::Image > textureThumb = thumbnailGenerator->get(
		fileName,
		64,
		64,
		visibleAlpha ? editor::IThumbnailGenerator::Alpha::WithAlpha : editor::IThumbnailGenerator::Alpha::NoAlpha,
		linearGamma ? editor::IThumbnailGenerator::Gamma::Linear : editor::IThumbnailGenerator::Gamma::SRGB
	);

	return textureThumb ? new ui::Bitmap(textureThumb) : nullptr;
}

}
