#include <stack>
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Flash/FlashBitmapImage.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashFont.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieRenderer.h"
#include "Flash/FlashOptimizer.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const SwfCxTransform c_cxfIdentity = { Color4f(1.0f, 1.0f, 1.0f, 1.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f) };

SwfCxTransform concateCxTransform(const SwfCxTransform& cxt1, const SwfCxTransform& cxt2)
{
	SwfCxTransform cxtr;
	cxtr.mul = cxt1.mul * cxt2.mul;
	cxtr.add = (cxt1.add * cxt2.mul + cxt2.add).saturated();
	return cxtr;
}

class MergeQueue
{
public:
	MergeQueue(FlashMovie* outputMovie, FlashFrame* outputFrame);

	void beginMask();

	void endMask();

	void endClip();

	void insertShape(const FlashShape* shape, const Matrix33& transform, const SwfCxTransform& cxform, uint8_t blendMode);

private:
	Ref< FlashMovie > m_outputMovie;
	RefArray< FlashFrame > m_outputFrame;
	Ref< FlashShape > m_mergeShape;
	Ref< FlashSprite > m_clippedSprite;
	Ref< FlashFrame > m_clippedFrame;
	Ref< FlashSprite > m_maskSprite;
	Ref< FlashFrame > m_maskFrame;
	std::map< const FlashShape*, uint32_t > m_usedIds;
	int32_t m_nextShapeId;
	int32_t m_nextDepth;
	std::stack< int32_t > m_maskDepths;
	uint8_t m_lastBlendMode;

	uint16_t cloneShape(const FlashShape& shape);
};

MergeQueue::MergeQueue(FlashMovie* outputMovie, FlashFrame* outputFrame)
:	m_outputMovie(outputMovie)
,	m_nextShapeId(2)
,	m_nextDepth(1)
,	m_lastBlendMode(0)
{
	m_outputFrame.push_back(outputFrame);
}

void MergeQueue::beginMask()
{
	T_FATAL_ASSERT(!m_maskSprite);

	int32_t clippedId = m_nextShapeId++;
	m_clippedFrame = new FlashFrame();
	m_clippedSprite = new FlashSprite(clippedId, 1);
	m_clippedSprite->addFrame(m_clippedFrame);
	m_outputMovie->defineCharacter(clippedId, m_clippedSprite);

	FlashFrame::PlaceObject place;
	place.hasFlags = FlashFrame::PfHasCharacterId;
	place.depth = m_nextDepth++;
	place.characterId = clippedId;
	place.clipDepth = 0;
	m_outputFrame.back()->placeObject(place);

	m_outputFrame.push_back(m_clippedFrame);

	int32_t maskId = m_nextShapeId++;
	m_maskFrame = new FlashFrame();
	m_maskSprite = new FlashSprite(maskId, 1);
	m_maskSprite->addFrame(m_maskFrame);
	m_outputMovie->defineCharacter(maskId, m_maskSprite);
	
	place.hasFlags = FlashFrame::PfHasCharacterId | FlashFrame::PfHasClipDepth;
	place.depth = m_nextDepth++;
	place.characterId = maskId;
	place.clipDepth = 0;
	m_outputFrame.back()->placeObject(place);

	m_maskDepths.push(place.depth);
}

void MergeQueue::endMask()
{
	T_FATAL_ASSERT(m_maskFrame);

	m_maskSprite = 0;
	m_maskFrame = 0;
	m_mergeShape = 0;
}

void MergeQueue::endClip()
{
	T_FATAL_ASSERT(!m_maskDepths.empty());
	T_FATAL_ASSERT(!m_outputFrame.empty());

	int32_t maskDepth = m_maskDepths.top();
	m_maskDepths.pop();

	FlashFrame::PlaceObject place = m_outputFrame.back()->getPlaceObjects()[maskDepth];
	T_FATAL_ASSERT (place.has(FlashFrame::PfHasClipDepth));
	place.clipDepth = m_nextDepth++;
	m_outputFrame.back()->placeObject(place);

	m_outputFrame.pop_back();
	T_FATAL_ASSERT(!m_outputFrame.empty());

	m_clippedSprite = 0;
	m_clippedFrame = 0;
	m_mergeShape = 0;
}

void MergeQueue::insertShape(const FlashShape* shape, const Matrix33& transform, const SwfCxTransform& cxform, uint8_t blendMode)
{
	if (m_maskFrame)
	{
		FlashFrame::PlaceObject place;
		place.hasFlags = FlashFrame::PfHasCharacterId | FlashFrame::PfHasMatrix;
		place.depth = m_nextDepth++;
		place.characterId = cloneShape(*shape);
		place.matrix = transform;
		m_maskFrame->placeObject(place);
	}
	else
	{
		if (blendMode != m_lastBlendMode)
			m_mergeShape = 0;

		if (!m_mergeShape)
		{
			m_mergeShape = new FlashShape(m_nextShapeId++);
			m_outputMovie->defineCharacter(m_mergeShape->getId(), m_mergeShape);

			FlashFrame::PlaceObject place;
			place.hasFlags = FlashFrame::PfHasCharacterId | FlashFrame::PfHasBlendMode;
			place.depth = m_nextDepth++;
			place.characterId = m_mergeShape->getId();
			place.blendMode = blendMode;
			m_outputFrame.back()->placeObject(place);
		}

		m_mergeShape->merge(*shape, transform, cxform);
		m_lastBlendMode = blendMode;
	}
}

uint16_t MergeQueue::cloneShape(const FlashShape& shape)
{
	std::map< const FlashShape*, uint32_t >::const_iterator i = m_usedIds.find(&shape);
	if (i != m_usedIds.end())
		return i->second;

	Ref< FlashShape > outputShape = new FlashShape(shape);
	m_outputMovie->defineCharacter(m_nextShapeId, outputShape);
		
	m_usedIds.insert(std::make_pair(&shape, m_nextShapeId));
	return m_nextShapeId++;
}

void traverse(MergeQueue& queue, const FlashMovie* movie, const FlashSprite* sprite, const Matrix33& transform, const SwfCxTransform& cxform, uint8_t blendMode)
{
	const FlashFrame* frame = sprite->getFrame(0);
	if (!frame)
		return;

	const SmallMap< uint16_t, Ref< FlashCharacter > >& characters = movie->getCharacters();
	const SmallMap< uint16_t, FlashFrame::PlaceObject >& placeObjects = frame->getPlaceObjects();

	bool clipEnable = false;
	int32_t clipDepth = -1;

	for (SmallMap< uint16_t, FlashFrame::PlaceObject >::const_iterator i = placeObjects.begin(); i != placeObjects.end(); ++i)
	{
		const FlashFrame::PlaceObject& placeObject = i->second;

		if (clipEnable)
		{
			if (placeObject.depth > clipDepth)
			{
				queue.endClip();
				clipEnable = false;
			}
		}

		if (placeObject.has(FlashFrame::PfHasMove) || placeObject.has(FlashFrame::PfHasCharacterId))
		{
			SwfCxTransform childCxForm = c_cxfIdentity;
			Matrix33 childTransform = Matrix33::identity();

			if (placeObject.has(FlashFrame::PfHasCxTransform))
				childCxForm = placeObject.cxTransform;

			if (placeObject.has(FlashFrame::PfHasMatrix))
				childTransform = placeObject.matrix;

			if (placeObject.has(FlashFrame::PfHasBlendMode))
				blendMode = placeObject.blendMode;

			if (placeObject.has(FlashFrame::PfHasCharacterId))
			{
				if (placeObject.has(FlashFrame::PfHasClipDepth))
				{
					clipEnable = true;
					clipDepth = placeObject.clipDepth;
					queue.beginMask();
				}

				SmallMap< uint16_t, Ref< FlashCharacter > >::const_iterator it = characters.find(placeObject.characterId);
				if (it != characters.end())
				{
					const FlashCharacter* character = it->second;
					T_FATAL_ASSERT (character);

					if (const FlashShape* childShape = dynamic_type_cast< const FlashShape* >(character))
						queue.insertShape(childShape, transform * childTransform, concateCxTransform(cxform, childCxForm), blendMode);
					else if (const FlashSprite* childSprite = dynamic_type_cast< const FlashSprite* >(character))
						traverse(queue, movie, childSprite, transform * childTransform, concateCxTransform(cxform, childCxForm), blendMode);
					else
						log::warning << L"Unable to merge character; type " << type_name(character) << L" not supported" << Endl;
				}

				if (placeObject.has(FlashFrame::PfHasClipDepth))
					queue.endMask();
			}
		}
	}

	if (clipEnable)
		queue.endClip();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashOptimizer", FlashOptimizer, Object)

Ref< FlashMovie > FlashOptimizer::merge(const FlashMovie* movie) const
{
	const FlashSprite* movieClip = movie->getMovieClip();
	if (!movieClip)
		return 0;

	Ref< FlashFrame > outputFrame = new FlashFrame();

	Ref< FlashSprite > outputSprite = new FlashSprite(1, 1);
	outputSprite->addFrame(outputFrame);

	Ref< FlashMovie > outputMovie = new FlashMovie(movie->getFrameBounds(), outputSprite);
	outputMovie->defineCharacter(1, outputSprite);

	// Copy bitmaps into output movie; is shared to reduce memory cost.
	const SmallMap< uint16_t, Ref< FlashBitmap > >& bitmaps = movie->getBitmaps();
	for (SmallMap< uint16_t, Ref< FlashBitmap > >::const_iterator i = bitmaps.begin(); i != bitmaps.end(); ++i)
		outputMovie->defineBitmap(i->first, i->second);

	MergeQueue queue(outputMovie, outputFrame);
	traverse(queue, movie, movieClip, Matrix33::identity(), c_cxfIdentity, SbmDefault);

	return outputMovie;
}

void FlashOptimizer::triangulate(FlashMovie* movie) const
{
	const SmallMap< uint16_t, Ref< FlashCharacter > >& characters = movie->getCharacters();
	for (SmallMap< uint16_t, Ref< FlashCharacter > >::const_iterator i = characters.begin(); i != characters.end(); ++i)
	{
		FlashShape* shape = dynamic_type_cast< FlashShape* >(i->second);
		if (shape)
		{
			shape->triangulate(false);
			shape->discardPaths();
		}
	}

	const SmallMap< uint16_t, Ref< FlashFont > >& fonts = movie->getFonts();
	for (SmallMap< uint16_t, Ref< FlashFont > >::const_iterator i = fonts.begin(); i != fonts.end(); ++i)
	{
		const RefArray< FlashShape >& glyphShapes = i->second->getShapes();
		for (RefArray< FlashShape >::const_iterator j = glyphShapes.begin(); j != glyphShapes.end(); ++j)
		{
			(*j)->triangulate(true);
			(*j)->discardPaths();
		}
	}
}

	}
}
