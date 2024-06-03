/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <stack>
#include "Core/Containers/SmallMap.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Spark/BitmapImage.h"
#include "Spark/Dictionary.h"
#include "Spark/Font.h"
#include "Spark/Frame.h"
#include "Spark/Movie.h"
#include "Spark/MovieRenderer.h"
#include "Spark/Optimizer.h"
#include "Spark/Shape.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"
#include "Spark/Swf/SwfTypes.h"

namespace traktor::spark
{
	namespace
	{

const ColorTransform c_cxfIdentity(Color4f(1.0f, 1.0f, 1.0f, 1.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f));

ColorTransform concateCxTransform(const ColorTransform& cxt1, const ColorTransform& cxt2)
{
	ColorTransform cxtr;
	cxtr.mul = cxt1.mul * cxt2.mul;
	cxtr.add = (cxt1.add * cxt2.mul + cxt2.add).saturated();
	return cxtr;
}

class MergeQueue
{
public:
	MergeQueue(Movie* outputMovie, Frame* outputFrame);

	void beginMask();

	void endMask();

	void endClip();

	void insertShape(const Shape* shape, const Matrix33& transform, const ColorTransform& cxform, uint8_t blendMode);

private:
	Ref< Movie > m_outputMovie;
	RefArray< Frame > m_outputFrame;
	Ref< Shape > m_mergeShape;
	Ref< Sprite > m_clippedSprite;
	Ref< Frame > m_clippedFrame;
	Ref< Sprite > m_maskSprite;
	Ref< Frame > m_maskFrame;
	SmallMap< const Shape*, uint32_t > m_usedIds;
	int32_t m_nextShapeId;
	int32_t m_nextDepth;
	std::stack< int32_t > m_maskDepths;
	uint8_t m_lastBlendMode;

	uint16_t cloneShape(const Shape& shape);
};

MergeQueue::MergeQueue(Movie* outputMovie, Frame* outputFrame)
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

	const int32_t clippedId = m_nextShapeId++;
	m_clippedFrame = new Frame();
	m_clippedSprite = new Sprite(1);
	m_clippedSprite->addFrame(m_clippedFrame);
	m_outputMovie->defineCharacter(clippedId, m_clippedSprite);

	Frame::PlaceObject place;
	place.hasFlags = Frame::PfHasCharacterId;
	place.depth = m_nextDepth++;
	place.characterId = clippedId;
	place.clipDepth = 0;
	m_outputFrame.back()->placeObject(place);

	m_outputFrame.push_back(m_clippedFrame);

	const int32_t maskId = m_nextShapeId++;
	m_maskFrame = new Frame();
	m_maskSprite = new Sprite(1);
	m_maskSprite->addFrame(m_maskFrame);
	m_outputMovie->defineCharacter(maskId, m_maskSprite);

	place.hasFlags = Frame::PfHasCharacterId | Frame::PfHasClipDepth;
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

	const int32_t maskDepth = m_maskDepths.top();
	m_maskDepths.pop();

	Frame::PlaceObject place = m_outputFrame.back()->getPlaceObjects()[maskDepth];
	T_FATAL_ASSERT (place.has(Frame::PfHasClipDepth));
	place.clipDepth = m_nextDepth++;
	m_outputFrame.back()->placeObject(place);

	m_outputFrame.pop_back();
	T_FATAL_ASSERT(!m_outputFrame.empty());

	m_clippedSprite = 0;
	m_clippedFrame = 0;
	m_mergeShape = 0;
}

void MergeQueue::insertShape(const Shape* shape, const Matrix33& transform, const ColorTransform& cxform, uint8_t blendMode)
{
	if (m_maskFrame)
	{
		Frame::PlaceObject place;
		place.hasFlags = Frame::PfHasCharacterId | Frame::PfHasMatrix;
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
			const int32_t mergeShapeId = m_nextShapeId++;

			m_mergeShape = new Shape();
			m_outputMovie->defineCharacter(mergeShapeId, m_mergeShape);

			Frame::PlaceObject place;
			place.hasFlags = Frame::PfHasCharacterId | Frame::PfHasBlendMode;
			place.depth = m_nextDepth++;
			place.characterId = mergeShapeId;
			place.blendMode = blendMode;
			m_outputFrame.back()->placeObject(place);
		}

		m_mergeShape->merge(*shape, transform, cxform);
		m_lastBlendMode = blendMode;
	}
}

uint16_t MergeQueue::cloneShape(const Shape& shape)
{
	auto it = m_usedIds.find(&shape);
	if (it != m_usedIds.end())
		return it->second;

	Ref< Shape > outputShape = new Shape(shape);
	m_outputMovie->defineCharacter(m_nextShapeId, outputShape);

	m_usedIds.insert(std::make_pair(&shape, m_nextShapeId));
	return m_nextShapeId++;
}

void traverse(MergeQueue& queue, const Movie* movie, const Sprite* sprite, const Matrix33& transform, const ColorTransform& cxform, uint8_t blendMode)
{
	const Frame* frame = sprite->getFrame(0);
	if (!frame)
		return;

	const SmallMap< uint16_t, Ref< Character > >& characters = movie->getCharacters();
	const SmallMap< uint16_t, Frame::PlaceObject >& placeObjects = frame->getPlaceObjects();

	bool clipEnable = false;
	int32_t clipDepth = -1;

	for (SmallMap< uint16_t, Frame::PlaceObject >::const_iterator i = placeObjects.begin(); i != placeObjects.end(); ++i)
	{
		const Frame::PlaceObject& placeObject = i->second;

		if (clipEnable)
		{
			if (placeObject.depth > clipDepth)
			{
				queue.endClip();
				clipEnable = false;
			}
		}

		if (placeObject.has(Frame::PfHasMove) || placeObject.has(Frame::PfHasCharacterId))
		{
			ColorTransform childCxForm = c_cxfIdentity;
			Matrix33 childTransform = Matrix33::identity();

			if (placeObject.has(Frame::PfHasCxTransform))
				childCxForm = placeObject.cxTransform;

			if (placeObject.has(Frame::PfHasMatrix))
				childTransform = placeObject.matrix;

			if (placeObject.has(Frame::PfHasBlendMode))
				blendMode = placeObject.blendMode;

			if (placeObject.has(Frame::PfHasCharacterId))
			{
				if (placeObject.has(Frame::PfHasClipDepth))
				{
					clipEnable = true;
					clipDepth = placeObject.clipDepth;
					queue.beginMask();
				}

				SmallMap< uint16_t, Ref< Character > >::const_iterator it = characters.find(placeObject.characterId);
				if (it != characters.end())
				{
					const Character* character = it->second;
					T_FATAL_ASSERT (character);

					if (const Shape* childShape = dynamic_type_cast< const Shape* >(character))
						queue.insertShape(childShape, transform * childTransform, concateCxTransform(cxform, childCxForm), blendMode);
					else if (const Sprite* childSprite = dynamic_type_cast< const Sprite* >(character))
						traverse(queue, movie, childSprite, transform * childTransform, concateCxTransform(cxform, childCxForm), blendMode);
					else
						log::warning << L"Unable to merge character; type " << type_name(character) << L" not supported" << Endl;
				}

				if (placeObject.has(Frame::PfHasClipDepth))
					queue.endMask();
			}
		}
	}

	if (clipEnable)
		queue.endClip();
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Optimizer", Optimizer, Object)

Ref< Movie > Optimizer::merge(const Movie* movie) const
{
	const Sprite* movieClip = movie->getMovieClip();
	if (!movieClip)
		return nullptr;

	Ref< Frame > outputFrame = new Frame();

	Ref< Sprite > outputSprite = new Sprite(1);
	outputSprite->addFrame(outputFrame);

	Ref< Movie > outputMovie = new Movie(movie->getFrameBounds(), outputSprite);
	outputMovie->defineCharacter(1, outputSprite);

	// Copy bitmaps into output movie; is shared to reduce memory cost.
	for (const auto& it : movie->getBitmaps())
		outputMovie->defineBitmap(it.first, it.second);

	MergeQueue queue(outputMovie, outputFrame);
	traverse(queue, movie, movieClip, Matrix33::identity(), c_cxfIdentity, SbmDefault);

	return outputMovie;
}

void Optimizer::triangulate(Movie* movie, bool discardPaths) const
{
	for (const auto& it : movie->getCharacters())
	{
		Shape* shape = dynamic_type_cast< Shape* >(it.second);
		if (shape)
		{
			shape->triangulate(false);
			if (discardPaths)
				shape->discardPaths();
		}
	}

	for (const auto& it : movie->getFonts())
	{
		for (auto shape : it.second->getShapes())
		{
			shape->triangulate(true);
			if (discardPaths)
				shape->discardPaths();
		}
	}
}

}
