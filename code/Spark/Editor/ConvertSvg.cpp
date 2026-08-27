/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spark/Editor/ConvertSvg.h"

#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Bezier2nd.h"
#include "Core/Math/Bezier3rd.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Spark/BitmapImage.h"
#include "Spark/Edit.h"
#include "Spark/Editor/ConvertFont.h"
#include "Spark/Editor/MovieAsset.h"
#include "Spark/Frame.h"
#include "Spark/Movie.h"
#include "Spark/Shape.h"
#include "Spark/Sprite.h"
#include "Svg/Document.h"
#include "Svg/ImageShape.h"
#include "Svg/IShapeVisitor.h"
#include "Svg/Parser.h"
#include "Svg/PathShape.h"
#include "Svg/Style.h"
#include "Svg/TextShape.h"
#include "Xml/Document.h"

#include <functional>

namespace traktor::spark
{
namespace
{

/*! Get name of character from SVG shape.
 *
 * SVG requires document unique "id" attributes, and editors such as Inkscape
 * silently rename duplicates, so the instance names the UI scripts look up
 * (background_mc, caption_tb, ...) cannot be expressed through "id" alone.
 * Therefore "inkscape:label" is used when available, "id" otherwise.
 */
std::wstring characterName(const svg::Shape* shape)
{
	const std::wstring label = shape->getAttribute(L"inkscape:label").getWideString();
	if (!label.empty())
		return label;
	else
		return shape->getAttribute(L"id").getWideString();
}

/*! Twice the signed area enclosed by an outline; negative when wound the other way. */
float doubleSignedArea(const AlignedVector< Vector2 >& outline)
{
	float area = 0.0f;
	for (size_t i = 0, n = outline.size(); i < n; ++i)
	{
		const Vector2& p0 = outline[i];
		const Vector2& p1 = outline[(i + 1) % n];
		area += p0.x * p1.y - p1.x * p0.y;
	}
	return area;
}

/*! Check if an outline enclose a point, by counting crossings of a ray cast from it. */
bool enclosing(const AlignedVector< Vector2 >& outline, const Vector2& pnt)
{
	if (outline.size() < 3)
		return false;

	bool inside = false;
	for (size_t i = 0, j = outline.size() - 1; i < outline.size(); j = i++)
	{
		const Vector2& p0 = outline[i];
		const Vector2& p1 = outline[j];
		if ((p0.y > pnt.y) != (p1.y > pnt.y))
		{
			const float x = (p1.x - p0.x) * (pnt.y - p0.y) / (p1.y - p0.y) + p0.x;
			if (pnt.x < x)
				inside = !inside;
		}
	}
	return inside;
}

class ShapeVisitor : public svg::IShapeVisitor
{
public:
	explicit ShapeVisitor(
		const std::function< bool(svg::Shape*) >& enter,
		const std::function< void(svg::Shape*) >& leave)
		: m_enter(enter)
		, m_leave(leave)
	{
	}

	virtual bool enter(svg::Shape* shape) override final
	{
		return m_enter(shape);
	}

	virtual void leave(svg::Shape* shape) override final
	{
		m_leave(shape);
	}

private:
	std::function< bool(svg::Shape*) > m_enter;
	std::function< void(svg::Shape*) > m_leave;
};

}

Ref< Movie > convertSvg(const traktor::Path& assetPath, const MovieAsset* movieAsset, const db::Instance* sourceInstance, IStream* sourceStream)
{
	xml::Document xd;
	if (!xd.loadFromStream(sourceStream))
	{
		log::error << L"Failed to import Spark movie; unable to read SVG." << Endl;
		return nullptr;
	}

	Ref< svg::Shape > shape = svg::Parser().parse(&xd);
	if (!shape)
	{
		log::error << L"Failed to import Spark movie; unable to parse SVG." << Endl;
		return nullptr;
	}

	svg::Document* document = dynamic_type_cast< svg::Document* >(shape);
	if (!document)
	{
		log::error << L"Failed to import Spark movie; no document node." << Endl;
		return nullptr;
	}

	const Vector2& movieSize = document->getSize() * 20.0f;
	const Aabb2& viewBox = document->getViewBox();

	// Create sprite for movie clip.
	Ref< Frame > movieFrame = new Frame();
	movieFrame->changeBackgroundColor(Color4f(0.5f, 0.5f, 0.5f, 1.0f));

	Ref< Sprite > movieSprite = new Sprite();
	movieSprite->addFrame(movieFrame);

	// Create movie container.
	Ref< Movie > movie = new Movie(Aabb2(Vector2(0.0f, 0.0f), Vector2(movieSize.x, movieSize.y)), movieSprite);

	// Import all fonts from the asset into the movie.
	SmallMap< std::wstring, uint16_t > fontIds;
	for (const auto& font : movieAsset->getFonts())
	{
		const uint16_t fontId = convertFont(assetPath, font, movie);
		if (fontId == 0)
			return nullptr;
		fontIds[toLower(font.name)] = fontId;
	}

	// Visit all shapes and create sprites and shapes.
	//
	// A sprite always has a base frame, frame 0, holding everything which isn't part
	// of a keyframe. "traktor:frame" groups add keyframes on top of that, the first
	// one sharing frame 0 so the sprite show it without having to be told to.
	struct SD
	{
		Ref< Sprite > sprite;
		Ref< Frame > baseFrame;			//!< Frame 0 of the sprite.
		Ref< Frame > frame;				//!< Frame which content is currently added to.
		Ref< Shape > shape;				//!< Paths accumulated for the frame being written.
		uint16_t nextDepth = 1;			//!< Depths are never reused across the frames of a sprite.
		uint16_t keyframes = 0;			//!< Number of keyframes added so far.
		uint16_t keyframeFirstDepth = 0;	//!< First depth used by the keyframe last entered.
		uint16_t keyframeEndDepth = 0;	//!< Depth beyond the last used by the keyframe last left.
	};

	AlignedVector< SD > spriteStack;
	uint32_t characterId = 1;
	uint16_t fillBitmapId = 1;

	// Map a point from the SVG view box into movie space.
	const auto toMovie = [&](const Matrix33& transform, const Vector2& pnt) {
		const Vector2 viewPnt = transform * pnt;						   // Point in view box.
		const Vector2 normPnt = (viewPnt - viewBox.mn) / viewBox.getSize(); // Normalized point.
		return normPnt * movieSize;										   // Point in movie.
	};

	// Place paths accumulated so far, if any, as a shape character onto the frame
	// being written; called whenever the target frame is about to change.
	const auto flushShape = [&](SD& sd) {
		if (sd.shape->getPaths().empty())
			return;

		movie->defineCharacter(characterId, sd.shape);

		Frame::PlaceObject p;
		p.hasFlags = Frame::PfHasCharacterId;
		p.depth = sd.nextDepth++;
		p.characterId = characterId;
		sd.frame->placeObject(p);

		characterId++;
		sd.shape = new Shape();
	};

	ShapeVisitor createCharactersVisitor(
		[&](svg::Shape* svg) -> bool {
		// Begin creating new sprite.
		if (svg->hasAttribute(L"traktor:sprite"))
		{
			const std::wstring id = characterName(svg);
			if (id.empty())
				return false;

			Ref< Sprite > sprite = new Sprite();
			Ref< Shape > shape = new Shape();

			movie->defineCharacter(characterId, sprite);

			if (spriteStack.empty())
				movie->setExport(wstombs(id), characterId);

			// Add frame and place shape.
			Ref< Frame > frame = new Frame();
			sprite->addFrame(frame);

			// Place this sprite on parent sprite.
			if (!spriteStack.empty())
			{
				Frame::PlaceObject p;
				p.hasFlags = Frame::PfHasName | Frame::PfHasCharacterId;
				p.depth = spriteStack.back().nextDepth++;
				p.name = wstombs(id);
				p.characterId = characterId;
				spriteStack.back().frame->placeObject(p);
			}

			// Add sprite to stack.
			spriteStack.push_back({ sprite, frame, frame, shape });
			log::info << L"Enter sprite \"" << id << L"\" (" << (characterId + 1) << L")..." << Endl;
			log::info << IncreaseIndent;

			characterId++;
		}

		// Begin a new keyframe of the sprite being created.
		if (svg->hasAttribute(L"traktor:frame") && !spriteStack.empty())
		{
			const std::wstring label = characterName(svg);
			if (label.empty())
				return false;

			SD& sd = spriteStack.back();

			// Everything up until now belong to the frame currently being written.
			flushShape(sd);

			if (sd.keyframes == 0)
			{
				// Let the first keyframe share frame 0 with the sprite's common content.
				sd.frame->setLabel(wstombs(label));
			}
			else
			{
				Ref< Frame > frame = new Frame();
				frame->setLabel(wstombs(label));
				sd.sprite->addFrame(frame);
				sd.frame = frame;

				// Frames are cumulative, so unless this frame explicitly removes what the
				// previous keyframe placed both would be shown when stepping forward.
				for (uint16_t depth = sd.keyframeFirstDepth; depth < sd.keyframeEndDepth; ++depth)
				{
					Frame::RemoveObject r;
					r.depth = depth;
					frame->removeObject(r);
				}
			}

			sd.keyframeFirstDepth = sd.nextDepth;
			sd.keyframes++;

			log::info << L"Enter frame \"" << label << L"\" (" << sd.keyframes << L")..." << Endl;
			log::info << IncreaseIndent;
		}

		if (!spriteStack.empty())
		{
			const Matrix33 transform = svg->getGlobalTransform();
			if (const auto ps = dynamic_type_cast< svg::PathShape* >(svg))
			{
				uint16_t fillStyle = 0;
				uint16_t lineStyle = 0;

				const auto style = ps->getStyle();
				if (style)
				{
					if (style->getFillEnable())
						fillStyle = spriteStack.back().shape->defineFillStyle(style->getFill() * Color4f(1.0f, 1.0f, 1.0f, style->getOpacity()));
					if (style->getStrokeEnable())
						lineStyle = spriteStack.back().shape->defineLineStyle(style->getStroke() * Color4f(1.0f, 1.0f, 1.0f, style->getOpacity()), (uint16_t)(style->getStrokeWidth() * 20.0f));
				}

				const auto& subPaths = ps->getPath().getSubPaths();
				if (subPaths.empty())
					return false;

				// Split the path into closed loops. svg::Path begin a new sub path on every
				// change of segment type, so a single loop is usually several sub paths; a
				// loop end at a closed sub path, or where the next one start somewhere else.
				AlignedVector< std::pair< uint32_t, uint32_t > > loops;
				for (uint32_t i = 0, begin = 0; i < (uint32_t)subPaths.size(); ++i)
				{
					if (i + 1 >= subPaths.size() || subPaths[i].closed || subPaths[i + 1].origin != subPaths[i].origin)
					{
						loops.push_back(std::make_pair(begin, i + 1));
						begin = i + 1;
					}
				}

				// Outline of each loop in movie space, so that a mirroring transform is taken
				// into account. Only on-curve points are needed; control points would bias an
				// area but never its sign, and never reach outside a loop enclosing them.
				AlignedVector< AlignedVector< Vector2 > > outlines(loops.size());
				for (uint32_t i = 0; i < (uint32_t)loops.size(); ++i)
				{
					for (uint32_t j = loops[i].first; j < loops[i].second; ++j)
					{
						const svg::SubPath& sp = subPaths[j];
						const uint32_t step = (sp.type == svg::SubPathType::Quadric) ? 2 : ((sp.type == svg::SubPathType::Cubic) ? 3 : 1);
						for (uint32_t k = 0; k < (uint32_t)sp.points.size(); k += step)
							outlines[i].push_back(toMovie(transform, sp.points[k]));
					}
				}

				// Which side of an edge the fill sit on isn't in the SVG data; it follow from
				// the winding of the loop and how deeply the loop is nested. A loop enclosed
				// by an odd number of others is a hole, anything else is solid, and walking a
				// solid loop should keep the fill to the right the way the primitives (<rect>,
				// <circle>, ...) are emitted. A loop wound against that has its styles
				// swapped, without which it fills its outside instead -- which in practice
				// means it render nothing at all, and do so without any warning.
				AlignedVector< bool > swapStyles(loops.size(), false);
				for (uint32_t i = 0; i < (uint32_t)loops.size(); ++i)
				{
					if (outlines[i].empty())
						continue;

					uint32_t enclosedBy = 0;
					for (uint32_t j = 0; j < (uint32_t)loops.size(); ++j)
					{
						if (j != i && enclosing(outlines[j], outlines[i].front()))
							enclosedBy++;
					}

					const bool solid = ((enclosedBy & 1) == 0);
					const bool positive = (doubleSignedArea(outlines[i]) >= 0.0f);
					swapStyles[i] = (positive != solid);
				}

				Path path;
				for (uint32_t loop = 0; loop < (uint32_t)loops.size(); ++loop)
				{
					const uint16_t leftFillStyle = swapStyles[loop] ? fillStyle : 0;
					const uint16_t rightFillStyle = swapStyles[loop] ? 0 : fillStyle;

					// Position to close back to; every loop has its own, and svg::Path record it
					// on each of the sub paths it split the loop into.
					const Vector2 closePosition = toMovie(transform, subPaths[loops[loop].first].origin);

					for (uint32_t subPath = loops[loop].first; subPath < loops[loop].second; ++subPath)
					{
						const svg::SubPath& sp = subPaths[subPath];

						AlignedVector< Vector2 > pnts = sp.points;

						// Convert points into document coordinates.
						for (auto& pnt : pnts)
							pnt = toMovie(transform, pnt);

						const size_t ln = pnts.size();
						switch (sp.type)
						{
						case svg::SubPathType::Linear:
							{
								path.moveTo((int32_t)(pnts[0].x), (int32_t)(pnts[0].y), Path::CmAbsolute);
								for (size_t i = 1; i < ln; ++i)
									path.lineTo((int32_t)(pnts[i].x), (int32_t)(pnts[i].y), Path::CmAbsolute);
							}
							break;

						case svg::SubPathType::Quadric:
							{
								path.moveTo((int32_t)(pnts[0].x), (int32_t)(pnts[0].y), Path::CmAbsolute);
								for (size_t i = 1; i < ln; i += 2)
									path.quadraticTo(
										(int32_t)(pnts[i].x),
										(int32_t)(pnts[i].y),
										(int32_t)(pnts[i + 1].x),
										(int32_t)(pnts[i + 1].y),
										Path::CmAbsolute);
							}
							break;

						case svg::SubPathType::Cubic:
							{
								path.moveTo((int32_t)(pnts[0].x), (int32_t)(pnts[0].y), Path::CmAbsolute);
								for (size_t i = 1; i < ln; i += 3)
								{
									const Bezier3rd b(
										pnts[i - 1],
										pnts[i],
										pnts[i + 1],
										pnts[i + 2]);

									AlignedVector< Bezier2nd > b2s;
									b.approximate(
										1.0f,
										4,
										b2s);
									for (const auto& b2 : b2s)
										path.quadraticTo(
											(int32_t)(b2.cp1.x),
											(int32_t)(b2.cp1.y),
											(int32_t)(b2.cp2.x),
											(int32_t)(b2.cp2.y),
											Path::CmAbsolute);
								}
							}
							break;

						default:
							break;
						}

						if (sp.closed)
							path.lineTo((int32_t)(closePosition.x), (int32_t)(closePosition.y), Path::CmAbsolute);

						path.end(leftFillStyle, rightFillStyle, lineStyle);
					}
				}
				spriteStack.back().shape->addPath(path);
			}
			else if (const auto is = dynamic_type_cast< const svg::ImageShape* >(svg))
			{
				const drawing::Image* image = is->getImage();

				const int32_t width = image->getWidth() * 20;
				const int32_t height = image->getHeight() * 20;

				Vector2 pnts[2];
				pnts[0] = is->getPosition();
				pnts[1] = is->getPosition() + is->getSize();
				for (auto& pnt : pnts)
				{
					const Vector2 viewPnt = transform * pnt;							// Point in view box.
					const Vector2 normPnt = (viewPnt - viewBox.mn) / viewBox.getSize(); // Normalized point.
					const Vector2 moviePnt = normPnt * movieSize;						// Point in movie.
					pnt = moviePnt;
				}

				movie->defineBitmap(fillBitmapId, new BitmapImage(image));

				const float sx = (pnts[1].x - pnts[0].x) / width;
				const float sy = (pnts[1].y - pnts[0].y) / height;
				const uint16_t fillStyle = spriteStack.back().shape->defineFillStyle(fillBitmapId, Matrix33(sx * 20.0f, 0.0f, pnts[0].x, 0.0f, sy * 20.0f, pnts[0].y, 0.0f, 0.0f, 1.0f), true);

				Path path;
				path.moveTo(pnts[0].x, pnts[0].y, Path::CmAbsolute);
				path.lineTo(pnts[1].x, pnts[0].y, Path::CmAbsolute);
				path.lineTo(pnts[1].x, pnts[1].y, Path::CmAbsolute);
				path.lineTo(pnts[0].x, pnts[1].y, Path::CmAbsolute);
				path.lineTo(pnts[0].x, pnts[0].y, Path::CmAbsolute);
				path.end(fillStyle, fillStyle, 0);

				spriteStack.back().shape->addPath(path);

				++fillBitmapId;
			}
			else if (const auto ts = dynamic_type_cast< const svg::TextShape* >(svg))
			{
				const std::wstring id = characterName(ts);
				if (id.empty())
					return false;

				const std::wstring font = ts->getStyle()->getFontFamily();
				if (font.empty())
					return false;

				const auto it = fontIds.find(toLower(font));
				if (it == fontIds.end())
					return false;

				// Calculate transform.
				const Vector2 viewTextSize = ts->getBoundingBox().getSize();   // Size in view box.
				const Vector2 normTextSize = viewTextSize / viewBox.getSize(); // Normalized size.
				const Vector2 movieTextSize = normTextSize * movieSize;		   // Size in movie.

				// Create an edit field; most likely since text fields are static.
				Ref< Edit > edit = new Edit(
					it->second,										   // font id
					(uint16_t)(ts->getStyle()->getFontSize() * 20.0f), // font height
					Aabb2(Vector2(0.0f, 0.0f), movieTextSize),		   // textBounds
					ts->getStyle()->getFill(),
					255,		   // maxLength
					ts->getText(), // initialText
					StaLeft,
					0,	   // leftMargin
					0,	   // rightMargin
					0,	   // indent
					0,	   // leading
					true,  // readOnly
					false, // wordWrap
					false, // multiLine
					false, // password
					false  // renderHtml
				);
				movie->defineCharacter(characterId, edit);

				// Place edit field on sprite.
				Frame::PlaceObject p;
				p.hasFlags = Frame::PfHasName | Frame::PfHasCharacterId; // | Frame::PfHasMatrix;
				p.depth = spriteStack.back().nextDepth++;
				p.name = wstombs(id);
				p.characterId = characterId;
				// p.matrix = Matrix33(
				//	1.0f, 0.0f, moviePnt.x,
				//	0.0f, 1.0f, moviePnt.y,
				//	0.0f, 0.0f, 1.0f
				//);
				spriteStack.back().frame->placeObject(p);

				log::info << L"Added textfield \"" << id << L"\"." << Endl;
				characterId++;
			}
		}

		return true;
	},
		[&](svg::Shape* svg) {
		// End keyframe; anything following it belong to frame 0 again.
		if (svg->hasAttribute(L"traktor:frame") && !spriteStack.empty())
		{
			SD& sd = spriteStack.back();

			flushShape(sd);
			sd.keyframeEndDepth = sd.nextDepth;
			sd.frame = sd.baseFrame;

			log::info << DecreaseIndent;
			log::info << L"Leave frame \"" << characterName(svg) << L"\"..." << Endl;
		}

		if (svg->hasAttribute(L"traktor:sprite"))
		{
			const std::wstring id = characterName(svg);

			// Place remaining paths onto the frame of the sprite still being written.
			flushShape(spriteStack.back());

			log::info << DecreaseIndent;
			log::info << L"Leave sprite \"" << id << L"\"..." << Endl;

			spriteStack.pop_back();
		}
	});
	shape->visit(&createCharactersVisitor);

	// Place sprite character on first frame of the root.
	// {
	// 	Frame::PlaceObject p;
	// 	p.hasFlags = Frame::PfHasCharacterId;
	// 	p.depth = 1;
	// 	p.characterId = 5;
	// 	movieFrame->placeObject(p);
	// }

	return movie;
}

}
