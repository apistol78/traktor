#pragma optimize( "", off )

#include <functional>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Bezier2nd.h"
#include "Core/Math/Bezier3rd.h"
#include "Core/Misc/TString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Spark/BitmapImage.h"
#include "Spark/Edit.h"
#include "Spark/Frame.h"
#include "Spark/Movie.h"
#include "Spark/Shape.h"
#include "Spark/Sprite.h"
#include "Spark/Editor/ConvertFont.h"
#include "Spark/Editor/ConvertSvg.h"
#include "Spark/Editor/MovieAsset.h"
#include "Svg/Document.h"
#include "Svg/ImageShape.h"
#include "Svg/IShapeVisitor.h"
#include "Svg/Parser.h"
#include "Svg/PathShape.h"
#include "Svg/Style.h"
#include "Svg/TextShape.h"
#include "Xml/Document.h"

namespace traktor::spark
{
	namespace
	{

class ShapeVisitor : public svg::IShapeVisitor
{
public:
	explicit ShapeVisitor(
		const std::function< bool(svg::Shape*) >& enter,
		const std::function< void(svg::Shape*) >& leave
	)
	:	m_enter(enter)
	,	m_leave(leave)
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
	for (const auto& font : movieAsset->getFonts())
	{
		if (!convertFont(assetPath, font, movie))
			return false;
	}

	// Visit all shapes and create sprites and shapes.
	struct SD
	{
		Ref< Sprite > sprite;
		Ref< Frame > frame;
		Ref< Shape > shape;
	};
	AlignedVector< SD > spriteStack;
	uint32_t characterId = 1;

	ShapeVisitor createCharactersVisitor(
		[&](svg::Shape* svg) -> bool
		{
			// Begin creating new sprite.
			if (svg->hasAttribute(L"traktor:sprite"))
			{
				const std::wstring id = svg->getAttribute(L"id").getWideString();
				if (id.empty())
					return false;

				Ref< Sprite > sprite = new Sprite();
				Ref< Shape > shape = new Shape();

				movie->defineCharacter(characterId + 0, shape);
				movie->defineCharacter(characterId + 1, sprite);

				if (spriteStack.empty())
					movie->setExport(wstombs(id), characterId + 1);

				// Add frame and place shape.
				Ref< Frame > frame = new Frame();
				sprite->addFrame(frame);

				// Place the shape, which we're about to build, onto created sprite.
				{
					Frame::PlaceObject p;
					p.hasFlags = Frame::PfHasCharacterId;
					p.depth = frame->nextUnusedDepth();
					p.characterId = characterId + 0;
					frame->placeObject(p);
				}

				// Place this sprite on parent sprite.
				if (!spriteStack.empty())
				{
					Frame::PlaceObject p;
					p.hasFlags = Frame::PfHasName | Frame::PfHasCharacterId;
					p.depth = spriteStack.back().frame->nextUnusedDepth();
					p.name = wstombs(id);
					p.characterId = characterId + 1;
					spriteStack.back().frame->placeObject(p);
				}

				// Add sprite to stack.
				spriteStack.push_back({ sprite, frame, shape });
				log::info << L"Enter sprite \"" << id << L"\" (" << (characterId + 1) << L")..." << Endl;
				log::info << IncreaseIndent;

				characterId += 2;
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

					// Get close position; when path is closed.
					Vector2 closePosition = subPaths.front().points.front();
					{
						const Vector2 viewPnt = transform * closePosition;						// Point in view box.
						const Vector2 normPnt = (viewPnt - viewBox.mn) / viewBox.getSize();		// Normalized point.
						const Vector2 moviePnt = normPnt * movieSize;							// Point in movie.
						closePosition = moviePnt;
					}

					Path path;
					for (const auto& sp : ps->getPath().getSubPaths())
					{
						AlignedVector< Vector2 > pnts = sp.points;

						// Convert points into document coordinates.
						for (auto& pnt : pnts)
						{
							const Vector2 viewPnt = transform * pnt;								// Point in view box.
							const Vector2 normPnt = (viewPnt - viewBox.mn) / viewBox.getSize();		// Normalized point.
							const Vector2 moviePnt = normPnt * movieSize;							// Point in movie.
							pnt = moviePnt;
						}

						const size_t ln = pnts.size();
						switch (sp.type)
						{
						case svg::SubPathType::Linear:
						{
							log::info << L"linear " << ln << L" (" << (sp.closed ? L"closed" : L"open") << L")" << Endl;
							path.moveTo((int32_t)(pnts[0].x), (int32_t)(pnts[0].y), Path::CmAbsolute);
							for (size_t i = 1; i < ln; ++i)
								path.lineTo((int32_t)(pnts[i].x), (int32_t)(pnts[i].y), Path::CmAbsolute);
						}
						break;

						case svg::SubPathType::Quadric:
						{
							log::info << L"quadric " << ln << L" (" << (sp.closed ? L"closed" : L"open") << L")" << Endl;
							path.moveTo((int32_t)(pnts[0].x), (int32_t)(pnts[0].y), Path::CmAbsolute);
							for (size_t i = 1; i < ln; i += 2)
								path.quadraticTo(
									(int32_t)(pnts[i].x), (int32_t)(pnts[i].y),
									(int32_t)(pnts[i + 1].x), (int32_t)(pnts[i + 1].y),
									Path::CmAbsolute
								);
						}
						break;

						case svg::SubPathType::Cubic:
						{
							log::info << L"cubic " << ln << L" (" << (sp.closed ? L"closed" : L"open") << L")" << Endl;
							path.moveTo((int32_t)(pnts[0].x), (int32_t)(pnts[0].y), Path::CmAbsolute);
							for (size_t i = 1; i < ln; i += 3)
							{
								const Bezier3rd b(
									pnts[i - 1],
									pnts[i],
									pnts[i + 1],
									pnts[i + 2]
								);

								AlignedVector< Bezier2nd > b2s;
								b.approximate(
									1.0f,
									4,
									b2s
								);
								for (const auto& b2 : b2s)
								{
									path.quadraticTo(
										(int32_t)(b2.cp1.x), (int32_t)(b2.cp1.y),
										(int32_t)(b2.cp2.x), (int32_t)(b2.cp2.y),
										Path::CmAbsolute
									);
								}
							}
						}
						break;

						default:
							break;
						}

						if (sp.closed)
							path.lineTo((int32_t)(closePosition.x), (int32_t)(closePosition.y), Path::CmAbsolute);

						path.end(0, fillStyle, lineStyle);
					}
					spriteStack.back().shape->addPath(path);
				}
				else if (const auto is = dynamic_type_cast< const svg::ImageShape* >(svg))
				{
					const drawing::Image* image = is->getImage();

					const int32_t width = image->getWidth() * 20;
					const int32_t height = image->getHeight() * 20;

					const uint16_t fillBitmap = 1;

					movie->defineBitmap(fillBitmap, new BitmapImage(image));

					const uint16_t fillStyle = spriteStack.back().shape->defineFillStyle(fillBitmap, Matrix33(
						20.0f, 0.0f, 0.0f,
						0.0f, 20.0f, 0.0f,
						0.0f, 0.0f, 1.0f
					), true);

					Path path;
					path.moveTo(0, 0, Path::CmAbsolute);
					path.lineTo(width, 0, Path::CmAbsolute);
					path.lineTo(width, height, Path::CmAbsolute);
					path.lineTo(0, height, Path::CmAbsolute);
					path.lineTo(0, 0, Path::CmAbsolute);
					path.end(fillStyle, fillStyle, 0);

					spriteStack.back().shape->addPath(path);
				}
				else if (const auto ts = dynamic_type_cast< const svg::TextShape* >(svg))
				{
					const std::wstring id = ts->getAttribute(L"id").getWideString();
					if (id.empty())
						return false;

					// Import font.
					const std::wstring font = ts->getStyle()->getFontFamily();
					if (font.empty())
						return false;

					// Calculate transform.
					const Vector2 viewPnt = Vector2(500.0f, 60.0f);				// Point in view box.
					const Vector2 normPnt = (viewPnt - viewBox.mn) / viewBox.getSize();		// Normalized point.
					const Vector2 moviePnt = normPnt * movieSize;							// Point in movie.

					float width = moviePnt.x; //  1305.0f;
					float height = moviePnt.y; //  325.0f;




					// Create an edit field; most likely since text fields are static.
					Ref< Edit > edit = new Edit(
						1,	// font id
						(uint16_t)(ts->getStyle()->getFontSize() * 20.0f),	// font height
						Aabb2(Vector2(0.0f, 0.0f), Vector2(width, height)),	// textBounds
						ts->getStyle()->getFill(),
						255,		// maxLength
						ts->getText(),	// initialText
						StaLeft,
						0,	// leftMargin
						0,	// rightMargin
						0,	// indent
						0,	// leading
						false,	// readOnly
						false,	// wordWrap
						false,	// multiLine
						false,	// password
						false	// renderHtml
					);
					movie->defineCharacter(characterId, edit);

					// Place edit field on sprite.
					Frame::PlaceObject p;
					p.hasFlags = Frame::PfHasName | Frame::PfHasCharacterId; // | Frame::PfHasMatrix;
					p.depth = spriteStack.back().frame->nextUnusedDepth();
					p.name = wstombs(id);
					p.characterId = characterId;
					//p.matrix = Matrix33(
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
		[&](svg::Shape* svg)
		{
			if (svg->hasAttribute(L"traktor:sprite"))
			{
				const std::wstring id = svg->getAttribute(L"id").getWideString();
				log::info << DecreaseIndent;
				log::info << L"Leave sprite \"" << id << L"\"..." << Endl;
				spriteStack.pop_back();
			}
		}
	);
	shape->visit(&createCharactersVisitor);



	// Place sprite character on first frame of the root.
	{
		Frame::PlaceObject p;
		p.hasFlags = Frame::PfHasCharacterId;
		p.depth = 1;
		p.characterId = 5;
		movieFrame->placeObject(p);
	}


	return movie;
}

}
