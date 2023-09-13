#pragma optimize( "", off )

#include <set>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_BBOX_H

#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Math/Bezier2nd.h"
#include "Core/Math/Bezier3rd.h"
#include "Core/Misc/TString.h"
#include "Database/Instance.h"
#include "Spark/Font.h"
#include "Spark/Movie.h"
#include "Spark/Shape.h"
#include "Spark/Editor/ConvertFont.h"
#include "Spark/Editor/MovieAsset.h"

namespace traktor::spark
{

uint16_t convertFont(const traktor::Path& assetPath, const MovieAsset::Font& fontAsset, Movie* movie)
{
	FT_Library library;
	FT_Face face;
	FT_Error error;

	const traktor::Path filePath = FileSystem::getInstance().getAbsolutePath(assetPath + fontAsset.fileName);

	error = FT_Init_FreeType(&library);
	if (error)
	{
		log::error << L"Unable to initialize FreeType library." << Endl;
		return 0;
	}

	error = FT_New_Face(
		library,
		wstombs(filePath.getPathNameOS()).c_str(),
		0,
		&face
	);
	if (error)
	{
		log::error << L"Unable to load font." << Endl;
		return 0;
	}

	error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	if (error)
	{
		log::error << L"Unable to select unicode char map." << Endl;
		return 0;
	}

	std::set< wchar_t > characters;

	// Insert Latin-1 code page.
	for (wchar_t ch = 0; ch < 256; ++ch)
		characters.insert(ch);

	// Scale factor from font to SWF em-square.
	const float scale = (1024.0f * 20.0f) / face->units_per_EM;

	const int16_t ascent = face->ascender * scale;
	const int16_t descent = -face->descender * scale;
	const int16_t leading = face->height * scale;

	RefArray< Shape > glyphShapes;
	AlignedVector< int16_t > advanceTable;
	AlignedVector< Aabb2 > boundsTable;
	SmallMap< uint32_t, int16_t > kerningLookup;
	AlignedVector< uint16_t > codeTable;

	for (auto ch : characters)
	{
		FT_UInt glyphIndex = FT_Get_Char_Index(face, ch);

		error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP);
		if (error)
		{
			log::warning << L"Unable to load glyph '" << ch << L"'." << Endl;
			continue;
		}

		FT_GlyphSlot slot = face->glyph;
		FT_Outline& outline = slot->outline;

		if (slot->format != FT_GLYPH_FORMAT_OUTLINE)
		{
			log::warning << L"Incorrect format on glyph slot." << Endl;
			continue;
		}
		if (outline.n_contours <= 0 || outline.n_points <= 0)
			continue;

		struct UD
		{
			Matrix33 transform;
			Path path;
		}
		ud;

		// Transformation.
		const Vector2 mn((float)face->bbox.xMin, (float)face->bbox.yMin);
		const Vector2 mx((float)face->bbox.xMax, (float)face->bbox.yMax);
		ud.transform = Matrix33(
			scale, 0.0f, 0.0f,
			0.0f, -scale, 0.0f,
			0.0f, 0.0f, 1.0f
		);

		// Create curves from font outline.
		FT_Outline_Funcs callbacks = {};
		callbacks.shift = 8;
		callbacks.delta = 0;

		callbacks.move_to = [](const FT_Vector* to, void* user) -> int {
			auto& ud = *(UD*)user;
			const Vector2 cp = ud.transform * Vector2((float)to->x / 256.0f, (float)to->y / 256.0f);
			ud.path.end(1, 0, 0);
			ud.path.moveTo(cp.x, cp.y, Path::CmAbsolute);
			return 0;
		};
		callbacks.line_to = [](const FT_Vector* to, void* user) -> int {
			auto& ud = *(UD*)user;
			const Vector2 cp2 = ud.transform * Vector2((float)to->x / 256.0f, (float)to->y / 256.0f);
			ud.path.lineTo(cp2.x, cp2.y, Path::CmAbsolute);
			return 0;
		};
		callbacks.conic_to = [](const FT_Vector* control, const FT_Vector* to, void* user) -> int {
			auto& ud = *(UD*)user;
			const Vector2 cp1 = ud.transform * Vector2((float)control->x / 256.0f, (float)control->y / 256.0f);
			const Vector2 cp2 = ud.transform * Vector2((float)to->x / 256.0f, (float)to->y / 256.0f);
			ud.path.quadraticTo(cp1.x, cp1.y, cp2.x, cp2.y, Path::CmAbsolute);
			return 0;
		};
		callbacks.cubic_to = [](const FT_Vector* controlOne, const FT_Vector* controlTwo, const FT_Vector* to, void* user) -> int {
			auto& ud = *(UD*)user;

			const Vector2 cp0 = ud.path.getCursor();
			const Vector2 cp1 = ud.transform * Vector2((float)controlOne->x / 256.0f, (float)controlOne->y / 256.0f);
			const Vector2 cp2 = ud.transform * Vector2((float)controlTwo->x / 256.0f, (float)controlTwo->y / 256.0f);
			const Vector2 cp3 = ud.transform * Vector2((float)to->x / 256.0f, (float)to->y / 256.0f);

			AlignedVector< Bezier2nd > b2v;
			Bezier3rd(cp0, cp1, cp2, cp3).approximate(1.5f, 2, b2v);

			for (const auto& it : b2v)
				ud.path.quadraticTo(it.cp1.x, it.cp1.y, it.cp2.x, it.cp2.y, Path::CmAbsolute);

			return 0;
		};
		FT_Outline_Decompose(&outline, &callbacks, (void*)&ud);

		ud.path.end(1, 0, 0);

		// Glyph advancement.
		advanceTable.push_back(slot->metrics.horiAdvance * scale);

		// Save bounding box.
		//FT_BBox boundingBox;
		//FT_Outline_Get_BBox(&outline, &boundingBox);
		//const Vector2 mn((float)boundingBox.xMin, (float)boundingBox.yMin);
		//const Vector2 mx((float)boundingBox.xMax, (float)boundingBox.yMax);
		boundsTable.push_back(Aabb2(Vector2::zero(), Vector2::zero())); //  mn* scale, mx* scale));

		const Aabb2 bounds = ud.path.getBounds();
		log::info << bounds.mn.x << L", " << bounds.mn.y << Endl;

		// Glyph code.
		codeTable.push_back(ch);

		// Convert into Spark shape.
		Ref< Shape > glyphShape = new Shape();
		glyphShape->addPath(ud.path);
		glyphShapes.push_back(glyphShape);
	}

	Ref< Font > font = new Font();
	font->initialize(
		wstombs(fontAsset.name),
		false,
		false,
		glyphShapes,
		ascent,
		descent,
		leading,
		advanceTable,
		boundsTable,
		kerningLookup,
		codeTable,
		Font::CtEMSquare
	);

	const uint16_t fontId = movie->nextFontId();

	movie->defineFont(
		fontId,
		font
	);

	return fontId;
}

}
