/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/Log/Log.h"
#include "Flash/Dictionary.h"
#include "Flash/DisplayList.h"
#include "Flash/Character.h"
#include "Flash/Frame.h"
#include "Flash/ICharacterFactory.h"
#include "Flash/Action/ActionContext.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const int32_t c_depthOffset = -16384;

struct FindCharacter
{
	const CharacterInstance* m_instance;

	FindCharacter(const CharacterInstance* instance)
	:	m_instance(instance)
	{
	}

	bool operator () (const std::pair< int32_t, DisplayList::Layer >& it) const
	{
		return it.second.instance == m_instance;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.DisplayList", DisplayList, Object)

DisplayList::DisplayList(ActionContext* context)
:	m_context(context)
{
	reset();
}

void DisplayList::reset()
{
	m_backgroundColor = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	m_layers.clear();
}

void DisplayList::updateBegin(bool reset)
{
	if (reset)
	{
		for (layer_map_t::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
			i->second.collect = true;
	}
}

void DisplayList::updateEnd()
{
	// Remove all layers which are still marked as "collect".
	for (layer_map_t::iterator i = m_layers.begin(); i != m_layers.end(); )
	{
		if (!i->second.immutable && i->second.collect)
		{
			if (i->second.instance)
			{
				m_context->getCharacterFactory()->removeInstance(i->second.instance, i->first);
				i->second.instance->clearCacheObject();
			}
			i = m_layers.erase(i);
		} 
		else
			i++;
	}
}

void DisplayList::updateFrame(CharacterInstance* ownerInstance, const Frame* frame)
{
	// Update background color.
	if (frame->hasBackgroundColorChanged())
		m_backgroundColor = frame->getBackgroundColor();

	// Remove instances from active list.
	const SmallMap< uint16_t, Frame::RemoveObject >& removeObjects = frame->getRemoveObjects();
	for (SmallMap< uint16_t, Frame::RemoveObject >::const_iterator i = removeObjects.begin(); i != removeObjects.end(); ++i)
	{
		const Frame::RemoveObject& removeObject = i->second;

		layer_map_t::iterator j = m_layers.find(removeObject.depth + c_depthOffset);
		if (j != m_layers.end())
		{
			if (removeObject.hasCharacterId)
			{
				if (j->second.id == removeObject.characterId)
				{
					if (j->second.instance)
					{
						m_context->getCharacterFactory()->removeInstance(j->second.instance, j->first);
						j->second.instance->clearCacheObject();
					}
					m_layers.erase(j);
				}
			}
			else
			{
				if (j->second.instance)
				{
					m_context->getCharacterFactory()->removeInstance(j->second.instance, j->first);
					j->second.instance->clearCacheObject();
				}
				m_layers.erase(j);
			}
		}
#if defined(_DEBUG)
		else
		{
			if (removeObject.hasCharacterId)
				log::warning << L"Unable to find character " << removeObject.characterId << L" in dictionary (1)" << Endl;
		}
#endif
	}

	// Place instances onto active list.
	const SmallMap< uint16_t, Frame::PlaceObject >& placeObjects = frame->getPlaceObjects();
	for (SmallMap< uint16_t, Frame::PlaceObject >::const_iterator i = placeObjects.begin(); i != placeObjects.end(); ++i)
	{
		const Frame::PlaceObject& placeObject = i->second;
		if (placeObject.has(Frame::PfHasMove) || placeObject.has(Frame::PfHasCharacterId))
		{
			int32_t depth = placeObject.depth + c_depthOffset;
			Layer& layer = m_layers[depth];

			if (placeObject.has(Frame::PfHasCharacterId) && placeObject.characterId != layer.id)
			{
				if (layer.instance)
				{
					m_context->getCharacterFactory()->removeInstance(layer.instance, depth);
					layer.instance->clearCacheObject();
				}

				Ref< const Character > character = ownerInstance->getDictionary()->getCharacter(placeObject.characterId);
				if (character)
				{
					// Use previous instance transform if we're replacing an instance.
					Matrix33 transform = layer.instance ? layer.instance->getTransform() : Matrix33::identity();

					// Create new instance.
					layer.id = placeObject.characterId;
					layer.instance = m_context->getCharacterFactory()->createInstance(
						character,
						depth,
						m_context,
						ownerInstance->getDictionary(),
						ownerInstance,
						placeObject.has(Frame::PfHasName) ? placeObject.name : "",
						placeObject.has(Frame::PfHasMatrix) ? placeObject.matrix : transform,
						0,
						&placeObject.events
					);
					T_ASSERT (layer.instance);
				}
#if defined(_DEBUG)
				else
					log::warning << L"Unable to find character " << placeObject.characterId << L" in dictionary (2)" << Endl;
#endif
			}

			if (!layer.instance)
				continue;

			if (placeObject.has(Frame::PfHasName))
				layer.instance->setName(placeObject.name);

			if (placeObject.has(Frame::PfHasCxTransform))
				layer.instance->setColorTransform(placeObject.cxTransform);

			if (placeObject.has(Frame::PfHasMatrix))
				layer.instance->setTransform(placeObject.matrix);

			if (placeObject.has(Frame::PfHasFilters))
			{
				layer.instance->setFilter(placeObject.filter);
				layer.instance->setFilterColor(placeObject.filterColor);
			}

			if (placeObject.has(Frame::PfHasBlendMode))
				layer.instance->setBlendMode(placeObject.blendMode);

			if (placeObject.has(Frame::PfHasVisible))
				layer.instance->setVisible(placeObject.visible != 0);

			if (placeObject.has(Frame::PfHasClipDepth))
			{
				layer.clipEnable = true;
				layer.clipDepth = placeObject.clipDepth + c_depthOffset;
			}

			if (!layer.instance->getName().empty())
				layer.name = m_context->getString(layer.instance->getName());
			else
				layer.name = ActionContext::IdEmpty;

			layer.immutable = false;
			layer.collect = false;
		}
		else
		{
			T_ASSERT (!placeObject.has(Frame::PfHasName));
			T_ASSERT (!placeObject.has(Frame::PfHasCxTransform));
			T_ASSERT (!placeObject.has(Frame::PfHasMatrix));

			layer_map_t::iterator j = m_layers.find(placeObject.depth + c_depthOffset);
			if (j != m_layers.end())
			{
				if (j->second.instance)
				{
					m_context->getCharacterFactory()->removeInstance(j->second.instance, j->first);
					j->second.instance->clearCacheObject();
				}
				m_layers.erase(j);
			}
		}
	}
}

void DisplayList::showObject(int32_t depth, uint16_t characterId, CharacterInstance* characterInstance, bool immutable)
{
	T_ASSERT (characterInstance);

	Layer& layer = m_layers[depth];

	if (layer.instance)
	{
		m_context->getCharacterFactory()->removeInstance(layer.instance, depth);
		layer.instance->clearCacheObject();
	}

	layer.id = characterId;
	layer.name = m_context->getString(characterInstance->getName());
	layer.instance = characterInstance;
	layer.immutable = immutable;
}

bool DisplayList::removeObject(CharacterInstance* characterInstance)
{
	T_ASSERT (characterInstance);

	layer_map_t::iterator i = std::find_if(m_layers.begin(), m_layers.end(), FindCharacter(characterInstance));
	if (i == m_layers.end())
		return false;

	m_context->getCharacterFactory()->removeInstance(characterInstance, i->first);
	characterInstance->clearCacheObject();

	m_layers.erase(i);
	return true;
}

bool DisplayList::removeObject(int32_t depth)
{
	layer_map_t::iterator i = m_layers.find(depth);
	if (i == m_layers.end())
		return false;

	if (i->second.instance)
	{
		m_context->getCharacterFactory()->removeInstance(i->second.instance, i->first);
		i->second.instance->clearCacheObject();
	}

	m_layers.erase(i);
	return true;
}

int32_t DisplayList::getObjectDepth(const CharacterInstance* characterInstance) const
{
	T_ASSERT (characterInstance);
	for (layer_map_t::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		if (i->second.instance == characterInstance)
			return i->first;
	}
	return 0;
}

int32_t DisplayList::getNextHighestDepth() const
{
	int32_t depth = 0;
	for (layer_map_t::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		depth = max(depth, i->first + 1);
	return depth;
}

int32_t DisplayList::getNextHighestDepthInRange(int32_t minDepth, int32_t maxDepth) const
{
	int32_t depth = minDepth;
	for (layer_map_t::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		if (i->first + 1 <= maxDepth)
			depth = max(depth, i->first + 1);
	}
	return depth;
}

void DisplayList::swap(int32_t depth1, int32_t depth2)
{
	layer_map_t::iterator it1 = m_layers.find(depth1);
	layer_map_t::iterator it2 = m_layers.find(depth2);

	if (it1 != m_layers.end() && it2 != m_layers.end())
	{
		std::swap(it1->second, it2->second);
	}
	else if (it1 != m_layers.end())
	{
		Layer layer = it1->second;
		m_layers.erase(it1);
		m_layers[depth2] = layer;
	}
	else if (it2 != m_layers.end())
	{
		Layer layer = it2->second;
		m_layers.erase(it2);
		m_layers[depth1] = layer;
	}
}

void DisplayList::getObjects(RefArray< CharacterInstance >& outCharacterInstances) const
{
	T_ASSERT (outCharacterInstances.empty());
	for (DisplayList::layer_map_t::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		T_ASSERT (i->second.instance);
		outCharacterInstances.push_back(i->second.instance);
	}
}

	}
}
