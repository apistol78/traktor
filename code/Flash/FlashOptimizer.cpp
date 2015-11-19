#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Flash/FlashBitmapData.h"
#include "Flash/FlashDictionary.h"
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

const SwfCxTransform c_cxfIdentity = { { 1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f } };

SwfCxTransform concateCxTransform(const SwfCxTransform& cxt1, const SwfCxTransform& cxt2)
{
	SwfCxTransform cxtr = 
	{
		{ cxt1.red[0]   * cxt2.red[0]  , clamp(cxt1.red[1]   * cxt2.red[0]   + cxt2.red[1],   0.0f, 1.0f) },
		{ cxt1.green[0] * cxt2.green[0], clamp(cxt1.green[1] * cxt2.green[0] + cxt2.green[1], 0.0f, 1.0f) },
		{ cxt1.blue[0]  * cxt2.blue[0] , clamp(cxt1.blue[1]  * cxt2.blue[0]  + cxt2.blue[1],  0.0f, 1.0f) },
		{ cxt1.alpha[0] * cxt2.alpha[0], clamp(cxt1.alpha[1] * cxt2.alpha[0] + cxt2.alpha[1], 0.0f, 1.0f) }
	};
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
	Ref< FlashFrame > m_outputFrame;
	Ref< FlashShape > m_mergeShape;
	Ref< FlashSprite > m_maskSprite;
	Ref< FlashFrame > m_maskFrame;
	std::map< const FlashShape*, uint32_t > m_usedIds;
	int32_t m_nextShapeId;
	int32_t m_nextDepth;
	int32_t m_maskDepth;

	uint16_t cloneShape(const FlashShape& shape);
};

MergeQueue::MergeQueue(FlashMovie* outputMovie, FlashFrame* outputFrame)
:	m_outputMovie(outputMovie)
,	m_outputFrame(outputFrame)
,	m_nextDepth(1)
,	m_nextShapeId(2)
,	m_maskDepth(-1)
{
}

void MergeQueue::beginMask()
{
	if (!m_maskSprite)
	{
		// Create a mask sprite; need to have a sprite because the mask can have multiple shapes
		// and how flash defines mask we must have a container of some sort.
		m_maskFrame = new FlashFrame();
		m_maskSprite = new FlashSprite(m_nextShapeId, 1);
		m_maskSprite->addFrame(m_maskFrame);
		m_outputMovie->defineCharacter(m_nextShapeId, m_maskSprite);
	
		// Place mask character onto output frame.
		FlashFrame::PlaceObject place;
		place.hasFlags = FlashFrame::PfHasCharacterId | FlashFrame::PfHasClipDepth;
		place.depth = m_nextDepth++;
		place.characterId = m_nextShapeId++;
		place.clipDepth = 0;
		m_outputFrame->placeObject(place);
	
		m_maskDepth = place.depth;
	}
}

void MergeQueue::endMask()
{
	m_maskSprite = 0;
	m_maskFrame = 0;
	m_mergeShape = 0;
}

void MergeQueue::endClip()
{
	if (m_maskDepth >= 0)
	{
		FlashFrame::PlaceObject place = m_outputFrame->getPlaceObjects()[m_maskDepth];
		T_FATAL_ASSERT (place.has(FlashFrame::PfHasClipDepth));
		place.clipDepth = m_nextDepth++;
		m_outputFrame->placeObject(place);
		m_maskDepth = -1;
		m_mergeShape = 0;
	}
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
		if (!m_mergeShape)
		{
			m_mergeShape = new FlashShape(m_nextShapeId++);
			m_outputMovie->defineCharacter(m_mergeShape->getId(), m_mergeShape);

			FlashFrame::PlaceObject place;
			place.hasFlags = FlashFrame::PfHasCharacterId | FlashFrame::PfHasBlendMode;
			place.depth = m_nextDepth++;
			place.characterId = m_mergeShape->getId();
			place.blendMode = blendMode;
			m_outputFrame->placeObject(place);
		}

		m_mergeShape->merge(*shape, transform, cxform);
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

	}
}
