#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Flash/FlashBitmapData.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieRenderer.h"
#include "Flash/FlashOptimizer.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/GC.h"
#include "Flash/IDisplayRenderer.h"
#include "Flash/Sw/SwDisplayRenderer.h"

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
	,	m_maskWrite(false)
	,	m_maskIncrement(false)
	,	m_maskDepth(0)
	{
	}

	virtual void begin(
		const FlashDictionary& dictionary,
		const SwfColor& backgroundColor,
		const Aabb2& frameBounds,
		float viewWidth,
		float viewHeight,
		const Vector4& viewOffset
	) T_OVERRIDE T_FINAL
	{
		// Copy bitmaps into output movie; is in fact shared to reduce memory cost.
		const SmallMap< uint16_t, Ref< FlashBitmap > >& bitmaps = dictionary.getBitmaps();
		for (SmallMap< uint16_t, Ref< FlashBitmap > >::const_iterator i = bitmaps.begin(); i != bitmaps.end(); ++i)
			m_outputMovie->defineBitmap(i->first, i->second);
	}

	virtual void beginSprite(const FlashSpriteInstance& sprite, const Matrix33& transform) T_OVERRIDE T_FINAL
	{
	}

	virtual void endSprite(const FlashSpriteInstance& sprite, const Matrix33& transform) T_OVERRIDE T_FINAL
	{
	}

	virtual void beginMask(bool increment) T_OVERRIDE T_FINAL
	{
		T_FATAL_ASSERT (!m_maskWrite);
		m_maskWrite = true;
		m_maskIncrement = increment;
		m_mergeShape = 0;
	}

	virtual void endMask() T_OVERRIDE T_FINAL
	{
		m_maskWrite = false;
		m_maskSprite = 0;
		m_maskFrame = 0;
		m_mergeShape = 0;
	}

	virtual void renderShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashShape& shape, const SwfCxTransform& cxform, uint8_t blendMode) T_OVERRIDE T_FINAL
	{
		if (m_maskWrite)
		{
			if (m_maskIncrement)
			{
				// Create a mask sprite; need to have a sprite because the mask can have multiple shapes
				// and how flash defines mask we must have a container of some sort.
				if (!m_maskSprite)
				{
					m_maskFrame = new FlashFrame();
					m_maskSprite = new FlashSprite(m_nextShapeId, 1);
					m_maskSprite->addFrame(m_maskFrame);

					m_outputMovie->defineCharacter(m_nextShapeId, m_maskSprite);

					// Place mask character onto output frame.
					FlashFrame::PlaceObject place;
					place.hasFlags = FlashFrame::PfHasCharacterId | FlashFrame::PfHasClipDepth;
					place.depth = m_nextDepth++;
					place.characterId = m_nextShapeId;
					place.clipDepth = 0;
					m_outputFrame->placeObject(place);

					m_maskDepth = place.depth;

					m_nextShapeId++;
				}

				// Place cloned character onto mask frame.
				FlashFrame::PlaceObject place;
				place.hasFlags = FlashFrame::PfHasCharacterId | FlashFrame::PfHasMatrix;
				place.depth = m_nextDepth++;
				place.characterId = cloneShape(shape);
				place.matrix = transform;
				m_maskFrame->placeObject(place);
			}
			else
			{
				// End of masking; patch clipping depth.
				FlashFrame::PlaceObject place = m_outputFrame->getPlaceObjects()[m_maskDepth];
				T_FATAL_ASSERT (place.has(FlashFrame::PfHasClipDepth));
				place.clipDepth = m_nextDepth - 1;
				m_outputFrame->placeObject(place);
			}
		}
		else
		{
			if (!m_mergeShape)
			{
				m_mergeShape = new FlashShape(m_nextShapeId++);
				m_outputMovie->defineCharacter(m_mergeShape->getId(), m_mergeShape);

				// Place cloned character onto frame.
				FlashFrame::PlaceObject place;
				place.hasFlags = FlashFrame::PfHasCharacterId | FlashFrame::PfHasBlendMode;
				place.depth = m_nextDepth++;
				place.characterId = m_mergeShape->getId();
				place.blendMode = blendMode;
				m_outputFrame->placeObject(place);
			}

			m_mergeShape->merge(shape, transform, cxform);
		}
	}

	virtual void renderMorphShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashMorphShape& shape, const SwfCxTransform& cxform) T_OVERRIDE T_FINAL
	{
	}

	virtual void renderGlyph(const FlashDictionary& dictionary, const Matrix33& transform, const Vector2& fontMaxDimension, const FlashShape& glyphShape, const SwfColor& color, const SwfCxTransform& cxform, uint8_t filter, const SwfColor& filterColor) T_OVERRIDE T_FINAL
	{
	}

	virtual void renderQuad(const Matrix33& transform, const Aabb2& bounds, const SwfCxTransform& cxform) T_OVERRIDE T_FINAL
	{
	}

	virtual void renderCanvas(const FlashDictionary& dictionary, const Matrix33& transform, const FlashCanvas& canvas, const SwfCxTransform& cxform) T_OVERRIDE T_FINAL
	{
	}

	virtual void end() T_OVERRIDE T_FINAL
	{
	}

private:
	Ref< FlashMovie > m_outputMovie;
	Ref< FlashFrame > m_outputFrame;
	std::map< const FlashShape*, uint32_t > m_usedIds;
	int32_t m_nextDepth;
	int32_t m_nextShapeId;
	Ref< FlashShape > m_mergeShape;
	bool m_maskWrite;
	bool m_maskIncrement;
	Ref< FlashSprite > m_maskSprite;
	Ref< FlashFrame > m_maskFrame;
	int32_t m_maskDepth;

	uint16_t cloneShape(const FlashShape& shape)
	{
		std::map< const FlashShape*, uint32_t >::const_iterator i = m_usedIds.find(&shape);
		if (i != m_usedIds.end())
			return i->second;

		Ref< FlashShape > outputShape = new FlashShape(shape);
		m_outputMovie->defineCharacter(m_nextShapeId, outputShape);
		
		m_usedIds.insert(std::make_pair(&shape, m_nextShapeId));
		return m_nextShapeId++;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashOptimizer", FlashOptimizer, Object)

Ref< FlashMovie > FlashOptimizer::merge(const FlashMovie* movie) const
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

	safeDestroy(movieInstance);
	return outputMovie;
}

Ref< FlashMovie > FlashOptimizer::rasterize(const FlashMovie* movie) const
{
	Ref< FlashSpriteInstance > movieInstance = movie->createMovieClipInstance(0);
	if (!movieInstance)
		return 0;

	Aabb2 bounds = movieInstance->getLocalBounds();
	int32_t width = int32_t(bounds.mx.x - bounds.mn.x);
	int32_t height = int32_t(bounds.mx.y - bounds.mn.y);
	
	int32_t pixelWidth = width / 20;
	int32_t pixelHeight = height / 20;

	// Rasterize first frame into an image.
	Ref< drawing::Image > image = new drawing::Image(drawing::PixelFormat::getA8R8G8B8(), pixelWidth, pixelHeight);
	image->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

	SwDisplayRenderer displayRenderer(image);
	FlashMovieRenderer movieRenderer(&displayRenderer);

	movieRenderer.renderFrame(
		movieInstance,
		bounds,
		width,
		height,
		Vector4(0.0f, 0.0f, 1.0f, 1.0f)
	);

	safeDestroy(movieInstance);
	GC::getInstance().collectCycles(true);

	static int32_t ccc = 0;
	image->save(L"flash_" + toString(ccc++) + L".png");

	// Generate a single shape from entire first frame.
	Ref< FlashShape > outputShape = new FlashShape(2);
	outputShape->create(1, width, height);

	Ref< FlashFrame > outputFrame = new FlashFrame();

	FlashFrame::PlaceObject place;
	place.hasFlags = FlashFrame::PfHasCharacterId;
	place.depth = 0;
	place.characterId = 2;
	outputFrame->placeObject(place);

	Ref< FlashSprite > outputSprite = new FlashSprite(1, 1);
	outputSprite->addFrame(outputFrame);

	Ref< FlashMovie > outputMovie = new FlashMovie(movie->getFrameBounds(), outputSprite);
	outputMovie->defineCharacter(1, outputSprite);
	outputMovie->defineBitmap(1, new FlashBitmapData(image));
	outputMovie->defineCharacter(2, outputShape);

	return outputMovie;
}

	}
}
