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
#include "Spark/Editor/ConvertSwf.h"
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
	ShapeVisitor(
		const std::function< void(svg::Shape*) >& enter,
		const std::function< void(svg::Shape*) >& leave
	)
	:	m_enter(enter)
	,	m_leave(leave)
	{
	}

	virtual void enter(svg::Shape* shape) override final
	{
		m_enter(shape);
	}

	virtual void leave(svg::Shape* shape) override final
	{
		m_leave(shape);
	}

private:
	std::function< void(svg::Shape*) > m_enter;
	std::function< void(svg::Shape*) > m_leave;
};

	}

Ref< Movie > convertSvg(const db::Instance* sourceInstance, IStream* sourceStream)
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

	const Vector2& size = document->getSize();
	const Aabb2& viewBox = document->getViewBox();

	// Create sprite for movie clip.
	Ref< Frame > movieFrame = new Frame();
	movieFrame->changeBackgroundColor(Color4f(1.0f, 1.0f, 1.0f, 1.0f));

	Ref< Sprite > movieSprite = new Sprite();
	movieSprite->addFrame(movieFrame);

	// Create movie container.
	Ref< Movie > movie = new Movie(Aabb2(Vector2(0.0f, 0.0f), Vector2(size.x * 20.0f, size.y * 20.0f)), movieSprite);

	// Create another sprite which contain the shape.
	Ref< Frame > shapeFrame = new Frame();
	Ref< Sprite > shapeSprite = new Sprite();
	shapeSprite->addFrame(shapeFrame);

	// Convert SVG shape into Spark shape.
	Ref< Shape > outputShape = new Shape();
	ShapeVisitor visitor(
		[&](svg::Shape* svg) {

			const Matrix33 transform = svg->getGlobalTransform();
			if (const auto ps = dynamic_type_cast< svg::PathShape* >(svg))
			{
				uint16_t fillStyle = 0;
				uint16_t lineStyle = 0;

				const auto style = ps->getStyle();
				if (style)
				{
					if (style->getFillEnable())
						fillStyle = outputShape->defineFillStyle(style->getFill() * Color4f(1.0f, 1.0f, 1.0f, style->getOpacity()));
					if (style->getStrokeEnable())
						lineStyle = outputShape->defineLineStyle(style->getStroke() * Color4f(1.0f, 1.0f, 1.0f, style->getOpacity()), (uint16_t)(style->getStrokeWidth() * 20.0f));
				}

				const auto& subPaths = ps->getPath().getSubPaths();
				if (subPaths.empty())
					return;

				// Get close position; when path is closed.
				Vector2 closePosition = subPaths.front().points.front();
				closePosition = (size * (transform * closePosition)) / viewBox.getSize();

				Path path;
				for (const auto& sp : ps->getPath().getSubPaths())
				{
					AlignedVector< Vector2 > pnts = sp.points;

					// Convert points into document coordinates.
					for (auto& pnt : pnts)
						pnt = (size * (transform * pnt)) / viewBox.getSize();

					const size_t ln = pnts.size();
					switch (sp.type)
					{
					case svg::SptLinear:
						{
							log::info << L"linear " << ln << L" (" << (sp.closed ? L"closed" : L"open") << L")" << Endl;
							path.moveTo((int32_t)(pnts[0].x * 20.0f), (int32_t)(pnts[0].y * 20.0f), Path::CmAbsolute);
							for (size_t i = 1; i < ln; ++i)
								path.lineTo((int32_t)(pnts[i].x * 20.0f), (int32_t)(pnts[i].y * 20.0f), Path::CmAbsolute);
						}
						break;

					case svg::SptQuadric:
						{
							log::info << L"quadric " << ln << L" (" << (sp.closed ? L"closed" : L"open") << L")" << Endl;
							path.moveTo((int32_t)(pnts[0].x * 20.0f), (int32_t)(pnts[0].y * 20.0f), Path::CmAbsolute);
							for (size_t i = 1; i < ln; i += 2)
								path.quadraticTo(
									(int32_t)(pnts[i].x * 20.0f), (int32_t)(pnts[i].y * 20.0f),
									(int32_t)(pnts[i + 1].x * 20.0f), (int32_t)(pnts[i + 1].y * 20.0f),
									Path::CmAbsolute
								);
						}
						break;

					case svg::SptCubic:
						{
							log::info << L"cubic " << ln << L" (" << (sp.closed ? L"closed" : L"open") << L")" << Endl;
							path.moveTo((int32_t)(pnts[0].x * 20.0f), (int32_t)(pnts[0].y * 20.0f), Path::CmAbsolute);
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
										(int32_t)(b2.cp1.x * 20.0f), (int32_t)(b2.cp1.y * 20.0f),
										(int32_t)(b2.cp2.x * 20.0f), (int32_t)(b2.cp2.y * 20.0f),
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
						path.lineTo((int32_t)(closePosition.x * 20.0f), (int32_t)(closePosition.y * 20.0f), Path::CmAbsolute);

					path.end(0, fillStyle, lineStyle);
				}
				outputShape->addPath(path);
			}
			else if (const auto is = dynamic_type_cast< const svg::ImageShape* >(svg))
			{
				const drawing::Image* image = is->getImage();

				const int32_t width = image->getWidth() * 20;
				const int32_t height = image->getHeight() * 20;

				const uint16_t fillBitmap = 1;

				movie->defineBitmap(fillBitmap, new BitmapImage(image));

				const uint16_t fillStyle = outputShape->defineFillStyle(fillBitmap, Matrix33(
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

				outputShape->addPath(path);
			}
			else if (const auto ts = dynamic_type_cast< const svg::TextShape* >(svg))
			{
				// Create an edit field; most likely since text fields are static.
				Ref< Edit > edit = new Edit(
					0,	// font id
					(uint16_t)ts->getStyle()->getFontSize(),	// font height
					Aabb2(),	// textBounds
					ts->getStyle()->getFill(),
					0,		// maxLength
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
				movie->defineCharacter(3, edit);

				// Place edit field on sprite.
				Frame::PlaceObject p;
				p.hasFlags = Frame::PfHasName | Frame::PfHasCharacterId;
				p.depth = 2;
				p.name = wstombs(ts->getId());
				p.characterId = 3;
				shapeFrame->placeObject(p);
			}
		},
		[&](svg::Shape*) {
		}
	);
	shape->visit(&visitor);

	// Place shape character on first frame of the sprite.
	{
		Frame::PlaceObject p;
		p.hasFlags = Frame::PfHasCharacterId;
		p.depth = 1;
		p.characterId = 1;
		shapeFrame->placeObject(p);
	}

	// Place sprite character on first frame of the root.
	{
		Frame::PlaceObject p;
		p.hasFlags = Frame::PfHasCharacterId;
		p.depth = 1;
		p.characterId = 2;
		movieFrame->placeObject(p);
	}

	// Add sprite to dictionary.
	movie->defineCharacter(1, outputShape);
	movie->defineCharacter(2, shapeSprite);
	movie->setExport(wstombs(sourceInstance->getName()), 2);
	return movie;
}

}
