#include "Flash/FlashFrame.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashOptimizer.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashSprite.h"


#include "Flash/FlashMovieRenderer.h"
#include "Flash/IDisplayRenderer.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

class CollectDisplayRenderer : public IDisplayRenderer
{
public:
	CollectDisplayRenderer(FlashMovie* outputMovie, FlashFrame* outputFrame)
	:	m_outputMovie(outputMovie)
	,	m_outputFrame(outputFrame)
	,	m_nextDepth(1)
	,	m_nextShapeId(2)
	{
	}

	virtual void begin(
		const FlashDictionary& dictionary,
		const SwfColor& backgroundColor,
		const Aabb2& frameBounds,
		float viewWidth,
		float viewHeight,
		const Vector4& viewOffset
	)
	{
	}

	virtual void beginMask(bool increment)
	{
	}

	virtual void endMask()
	{
	}

	virtual void renderShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashShape& shape, const SwfCxTransform& cxform, uint8_t blendMode)
	{
		// Clone shape, don't want cross references between input and output movie.
		Ref< FlashShape > outputShape = new FlashShape(shape);
		m_outputMovie->defineCharacter(m_nextShapeId, outputShape);

		// Place cloned character onto output frame.
		FlashFrame::PlaceObject place;
		place.hasFlags = FlashFrame::PfHasCharacterId | FlashFrame::PfHasMatrix | FlashFrame::PfHasCxTransform;
		place.depth = m_nextDepth;
		place.characterId = m_nextShapeId;
		place.matrix = transform;
		place.cxTransform = cxform;
		m_outputFrame->placeObject(place);

		++m_nextDepth;
		++m_nextShapeId;
	}

	virtual void renderMorphShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashMorphShape& shape, const SwfCxTransform& cxform)
	{
	}

	virtual void renderGlyph(const FlashDictionary& dictionary, const Matrix33& transform, const Vector2& fontMaxDimension, const FlashShape& glyphShape, const SwfColor& color, const SwfCxTransform& cxform, uint8_t filter, const SwfColor& filterColor)
	{
	}

	virtual void renderQuad(const Matrix33& transform, const Aabb2& bounds, const SwfCxTransform& cxform)
	{
	}

	virtual void renderCanvas(const FlashDictionary& dictionary, const Matrix33& transform, const FlashCanvas& canvas, const SwfCxTransform& cxform)
	{
	}

	virtual void end()
	{
	}

private:
	FlashMovie* m_outputMovie;
	FlashFrame* m_outputFrame;
	uint32_t m_nextDepth;
	uint32_t m_nextShapeId;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashOptimizer", FlashOptimizer, Object)

Ref< FlashMovie > FlashOptimizer::optimizeStaticMovie(const FlashMovie* movie) const
{
	Ref< FlashSpriteInstance > movieInstance = movie->createMovieClipInstance(0);
	if (!movieInstance)
		return 0;

	// Generate a single shape from entire first frame.
	Ref< FlashFrame > outputFrame = new FlashFrame();

	Ref< FlashSprite > outputSprite = new FlashSprite(1, 1);
	outputSprite->addFrame(outputFrame);

	Ref< FlashMovie > outputMovie = new FlashMovie(movie->getFrameBounds(), outputSprite);
	outputMovie->defineCharacter(1, outputSprite);

	CollectDisplayRenderer displayRenderer(outputMovie, outputFrame);
	FlashMovieRenderer movieRenderer(&displayRenderer);

	movieRenderer.renderFrame(
		movieInstance,
		movie->getFrameBounds(),
		1920.0f,
		1080.0f,
		Vector4(0.0f, 0.0f, 1.0f, 1.0f)
	);

	return outputMovie;
}

	}
}
