#include <ft2build.h>
#include FT_FREETYPE_H
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Misc/AutoPtr.h"
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

struct GlyphRect
{
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.FontPipeline", 1, FontPipeline, editor::IPipeline)

bool FontPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
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
	pipelineDepends->addDependency(Guid(L"{0B7B3724-0AC1-2C45-BD37-D809BEEBA2BC}"), editor::PdfBuild | editor::PdfResource);
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

#if defined(_WIN32)
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
#else
	// FIXME Linker error on clang
	return false;
#endif

	error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	if (error)
	{
		log::error << L"Font pipeline failed; unable to select unicode char map." << Endl;
		return false;
	}

	const int32_t c_downScaleFactor = 4;
	const int32_t c_searchDistance = 32;
	const int32_t c_glyphPixelSize = 512;
	const int32_t c_glyphOutputPixelSize = c_glyphPixelSize / c_downScaleFactor;

	FT_Set_Pixel_Sizes(
		face,
		0,
		c_glyphPixelSize
	);

	// Rasterize all glyphs.
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

		int32_t width = slot->bitmap.width;
		int32_t height = slot->bitmap.rows;

		// Insert the glyph into a source image.
		Ref< drawing::Image > source = new drawing::Image(drawing::PixelFormat::getR8(), width + c_searchDistance * 2, height + c_searchDistance * 2);
		source->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

		for (int32_t y = 0; y < height; ++y)
		{
			for (int32_t x = 0; x < width; ++x)
			{
				float f = slot->bitmap.buffer[x + y * slot->bitmap.width] / 255.0f;
				source->setPixel(x + c_searchDistance, y + c_searchDistance, Color4f(f, f, f, f));
			}
		}

		// Create output image.
		int32_t outputWidth = (width + c_searchDistance * 2) / c_downScaleFactor;
		int32_t outputHeight = (height + c_searchDistance * 2) / c_downScaleFactor;

		Ref< drawing::Image > output = new drawing::Image(
			drawing::PixelFormat::getR8(),
			outputWidth,
			outputHeight
		);

		// Calculate signed distance.
		Color4f p1, p2;
		for (int32_t y = 0; y < outputHeight; ++y)
		{
			int32_t sourceY = y * c_downScaleFactor;
			int32_t y0 = max(0, sourceY - c_searchDistance);
			int32_t y1 = min(source->getHeight() - 1, sourceY + c_searchDistance);

			for (int32_t x = 0; x < outputWidth; ++x)
			{
				int32_t sourceX = x * c_downScaleFactor;
				int32_t x0 = max(0, sourceX - c_searchDistance);
				int32_t x1 = min(source->getWidth() - 1, sourceX + c_searchDistance);

				source->getPixelUnsafe(sourceX, sourceY, p1);

				float D = c_searchDistance;
				for (int32_t sy = y0; sy <= y1; ++sy)
				{
					for (int32_t sx = x0; sx <= x1; ++sx)
					{
						source->getPixelUnsafe(sx, sy, p2);
						if (bool(p1.getRed() >= 0.5f) != bool(p2.getRed() >= 0.5f))
						{
							float dx = sx - sourceX;
							float dy = sy - sourceY;
							D = min(D, std::sqrt(dx * dx + dy * dy));
						}
					}
				}

				float f = D / float(c_searchDistance);
				if (p1.getRed() >= 0.5f)
					f = -f;

				f = 0.5f - f * 0.5f;

				output->setPixelUnsafe(x, y, Color4f(f, f, f, f));
			}
		}

		glyphImages[i] = output;

		fontResource->m_glyphs[i].ch = ch;
		fontResource->m_glyphs[i].offset[0] = float(slot->bitmap_left + c_searchDistance) / c_glyphPixelSize;
		fontResource->m_glyphs[i].offset[1] = float(slot->bitmap_top + c_searchDistance) / c_glyphPixelSize;
		fontResource->m_glyphs[i].advance = (float(slot->advance.x) / c_glyphPixelSize) / float(1 << 6);
	}

	// Calculate an estimate size of atlas.
	int32_t totalArea = 0;
	for (RefArray< drawing::Image >::const_iterator i = glyphImages.begin(); i != glyphImages.end(); ++i)
	{
		if (*i)
			totalArea += (*i)->getWidth() * (*i)->getHeight();
	}
	
	int32_t size = nearestLog2(std::max(int32_t(std::sqrt(totalArea) / 2.0f), 16));

	// Pack atlas of all glyphs.
	AutoPtr< stbrp_context > packer(new stbrp_context());
	AutoArrayPtr< stbrp_node > nodes(new stbrp_node [size]);

	stbrp_setup_allow_out_of_mem(packer.ptr(), 1);
	stbrp_init_target(packer.ptr(), size, size, nodes.ptr(), size);

	for (uint32_t i = 0; i < glyphImages.size(); )
	{
		if (!glyphImages[i])
		{
			++i;
			continue;
		}

		int32_t width = glyphImages[i]->getWidth();
		int32_t height = glyphImages[i]->getHeight();

		stbrp_rect r = { 0 };
		r.w = width + 2;
		r.h = height + 2;
		stbrp_pack_rects(packer.ptr(), &r, 1);
		if (!r.was_packed)
		{
			// Not enough space available in atlas, enlarge atlas and try again.
			i = 0;
			size <<= 1;

			packer.reset(new stbrp_context());
			nodes.reset(new stbrp_node [size]);

			stbrp_setup_allow_out_of_mem(packer.ptr(), 1);
			stbrp_init_target(packer.ptr(), size, size, nodes.ptr(), size);
			continue;
		}

		glyphRects[i].x = r.x + 1;
		glyphRects[i].y = r.y + 1;
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
		fontResource->m_glyphs[i].unit[0] = float(glyphRects[i].width) / c_glyphOutputPixelSize;
		fontResource->m_glyphs[i].unit[1] = float(glyphRects[i].height) / c_glyphOutputPixelSize;
	}

#if defined(_DEBUG)
	fontImage.save(L"FontImage.png");
#endif

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

	Guid fontTextureOutputGuid = outputGuid.permutate(1);
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
	fontResource->m_shader = resource::Id< render::Shader >(Guid(L"{0B7B3724-0AC1-2C45-BD37-D809BEEBA2BC}"));

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
