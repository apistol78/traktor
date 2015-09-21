#include <ft2build.h>
#include FT_FREETYPE_H

#include <MaxRectsBinPack.h>

#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Spark/FontResource.h"
#include "Spark/Editor/FontAsset.h"
#include "Spark/Editor/FontPipeline.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

Guid incrementGuid(const Guid& g)
{
	uint8_t d[16];
	for (int i = 0; i < 16; ++i)
		d[i] = g[i];
	reinterpret_cast< uint32_t& >(d[12])++;
	return Guid(d);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.FontPipeline", 0, FontPipeline, editor::IPipeline)

bool FontPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void FontPipeline::destroy()
{
}

TypeInfoSet FontPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< FontAsset >());
	return typeSet;
}

bool FontPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const FontAsset* fontAsset = mandatory_non_null_type_cast< const FontAsset* >(sourceAsset);
	pipelineDepends->addDependency(Path(m_assetPath), fontAsset->getFileName().getOriginal());
	pipelineDepends->addDependency(fontAsset->m_shader, editor::PdfBuild | editor::PdfResource);
	return true;
}

bool FontPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const FontAsset* fontAsset = mandatory_non_null_type_cast< const FontAsset* >(sourceAsset);

	FT_Library library;
	FT_Face face;
	FT_Error error;
	
	error = FT_Init_FreeType(&library);
	if (error)
	{
		log::error << L"Font pipeline failed; unable to initialize FreeType library." << Endl;
		return false;
	}

	Path fontFile = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + fontAsset->getFileName());

	error = FT_New_Face(
		library,
		wstombs(fontFile.getPathName()).c_str(),
		0,
		&face
	);
	if (error)
	{
		log::error << L"Font pipeline failed; unable to load font \"" << fontFile.getPathName() << L"\"." << Endl;
		return false;
	}

	error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	if (error)
	{
		log::error << L"Font pipeline failed; unable to select unicode char map." << Endl;
		return false;
	}

	const int32_t c_glyphPixelSize = 128;

	FT_Set_Pixel_Sizes(
		face,
		0,
		c_glyphPixelSize
	);

	// Rasterize all glyphs.
	struct GlyphRect { int32_t x; int32_t y; int32_t width; int32_t height; };

	Ref< FontResource > fontResource = new FontResource();
	fontResource->m_glyphs.resize(fontAsset->m_includeCharacters.length());

	RefArray< drawing::Image > glyphImages(fontAsset->m_includeCharacters.length());
	std::vector< GlyphRect > glyphRects(fontAsset->m_includeCharacters.length());

	for (uint32_t i = 0; i < fontAsset->m_includeCharacters.length(); ++i)
	{
		wchar_t ch = fontAsset->m_includeCharacters[i];
		FT_UInt glyphIndex = FT_Get_Char_Index(face, ch);

		error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
		if (error)
		{
			log::warning << L"Unable to load glyph '" << ch << L"'" << Endl;
			continue;
		}

		error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		if (error)
		{
			log::warning << L"Unable to render glyph '" << ch << L"'" << Endl;
			continue;
		}

		FT_GlyphSlot slot = face->glyph;

		int32_t width = slot->bitmap.width + slot->bitmap_left;
		int32_t height = slot->bitmap.rows;

		glyphImages[i] = new drawing::Image(drawing::PixelFormat::getR8(), width, height);
		glyphImages[i]->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

		for (int32_t y = 0; y < height; ++y)
		{
			for (int32_t x = 0; x < width; ++x)
			{
				float f = slot->bitmap.buffer[x + y * slot->bitmap.width] / 255.0f;
				glyphImages[i]->setPixel(x + slot->bitmap_left, y, Color4f(f, f, f, f));
			}
		}

		fontResource->m_glyphs[i].ch = ch;
		fontResource->m_glyphs[i].advance = (float(slot->advance.x) / c_glyphPixelSize) / float(1 << 6);

		/*
		// Transform kerning for this glyph compared to all other characters.
		for (uint32_t j = 0; j < fontAsset->m_includeCharacters.length(); ++j)
		{
			if (i == j)
				continue;

			wchar_t rightCh = fontAsset->m_includeCharacters[j];
			FT_UInt rightGlyphIndex = FT_Get_Char_Index(face, rightCh);

			FT_Vector kerning;
			error = FT_Get_Kerning(face, glyphIndex, rightGlyphIndex, FT_KERNING_DEFAULT, &kerning);
			if (error)
			{
				log::warning << L"Unable to get kerning for glyph '" << ch << L"'" << Endl;
				continue;
			}

			if (kerning.x == 0)
				continue;

			float kf = (float(slot->advance.x) / c_glyphPixelSize) / float(1 << 6);
			log::info << L"Kerning of '" << ch << L"' to '" << rightCh << L"' is " << kf << L" fractions" << Endl;
		}
		*/
	}

	// Calculate an estimate size of atlas.
	int32_t totalArea = 0;
	for (RefArray< drawing::Image >::const_iterator i = glyphImages.begin(); i != glyphImages.end(); ++i)
		totalArea += (*i)->getWidth() * (*i)->getHeight();
	
	int32_t size = nearestLog2(std::max(int32_t(std::sqrt(totalArea) / 2.0f), 16));

	// Pack atlas of all glyphs.
	rbp::MaxRectsBinPack binPack;
	binPack.Init(size, size);

	for (uint32_t i = 0; i < glyphImages.size(); )
	{
		if (!glyphImages[i])
			continue;

		int32_t width = glyphImages[i]->getWidth();
		int32_t height = glyphImages[i]->getHeight();
		int32_t packSize = std::max(width, height) + 2;

		rbp::Rect packedRect = binPack.Insert(packSize, packSize, rbp::MaxRectsBinPack::RectBestAreaFit);
		if (packedRect.height <= 0)
		{
			// Not enough space available in atlas, enlarge atlas and try again.
			i = 0;
			size <<= 1;
			binPack.Init(size, size);
			continue;
		}

		glyphRects[i].x = packedRect.x + 1;
		glyphRects[i].y = packedRect.y + 1;
		glyphRects[i].width = width;
		glyphRects[i].height = height;

		++i;
	}

	// Create atlas image of all glyphs.
	drawing::Image fontImage(
		drawing::PixelFormat::getR8(),
		size,
		size
	);
	fontImage.clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

	for (uint32_t i = 0; i < glyphImages.size(); ++i)
	{
		if (!glyphImages[i])
			continue;

		fontImage.copy(
			glyphImages[i],
			glyphRects[i].x,
			glyphRects[i].y,
			0,
			0,
			glyphRects[i].width,
			glyphRects[i].height
		);

		fontResource->m_glyphs[i].rect[0] = float(glyphRects[i].x) / size;
		fontResource->m_glyphs[i].rect[1] = float(glyphRects[i].y) / size;
		fontResource->m_glyphs[i].rect[2] = float(glyphRects[i].width) / size;
		fontResource->m_glyphs[i].rect[3] = float(glyphRects[i].height) / size;
		fontResource->m_glyphs[i].unit[0] = float(glyphRects[i].width) / c_glyphPixelSize;
		fontResource->m_glyphs[i].unit[1] = float(glyphRects[i].height) / c_glyphPixelSize;
	}

	fontImage.save(L"FontImage.png");

	Ref< render::TextureOutput > fontOutput = new render::TextureOutput();
	fontOutput->m_textureFormat = render::TfR8;
	fontOutput->m_generateNormalMap = false;
	fontOutput->m_scaleDepth = 0.0f;
	fontOutput->m_generateMips = true;
	fontOutput->m_keepZeroAlpha = false;
	fontOutput->m_textureType = render::Tt2D;
	fontOutput->m_hasAlpha = false;
	fontOutput->m_ignoreAlpha = false;
	fontOutput->m_scaleImage = false;
	fontOutput->m_scaleWidth = 0;
	fontOutput->m_scaleHeight = 0;
	fontOutput->m_enableCompression = false;
	fontOutput->m_enableNormalMapCompression = false;
	fontOutput->m_inverseNormalMapY = false;
	fontOutput->m_linearGamma = true;
	fontOutput->m_generateSphereMap = false;
	fontOutput->m_preserveAlphaCoverage = false;
	fontOutput->m_alphaCoverageReference = 0.0f;
	fontOutput->m_sharpenRadius = 0;
	fontOutput->m_sharpenStrength = 0.0f;
	fontOutput->m_systemTexture = true;

	Guid fontTextureOutputGuid = incrementGuid(outputGuid);
	std::wstring fontTextureOutputPath = traktor::Path(outputPath).getPathOnly() + L"/" + outputGuid.format() + L"/Texture";
	if (!pipelineBuilder->buildOutput(
		fontOutput,
		fontTextureOutputPath,
		fontTextureOutputGuid,
		&fontImage
	))
	{
		log::error << L"Font pipeline failed; unable to build font texture" << Endl;
		return false;
	}

	fontResource->m_texture = resource::Id< render::ISimpleTexture >(fontTextureOutputGuid);
	fontResource->m_shader = fontAsset->m_shader;

	// Create output instance.
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!outputInstance)
	{
		log::error << L"Font pipeline failed; unable to create output instance" << Endl;
		return false;
	}

	// Commit resource.
	outputInstance->setObject(fontResource);
	if (!outputInstance->commit())
	{
		log::error << L"Font pipeline failed; unable to commit output instance" << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > FontPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
