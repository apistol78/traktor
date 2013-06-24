#include <algorithm>
#include "Core/Log/Log.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashDisplayList.h"
#include "Flash/FlashCharacter.h"
#include "Flash/FlashCharacterInstance.h"
#include "Flash/FlashFrame.h"
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
	const FlashCharacterInstance* m_instance;

	FindCharacter(const FlashCharacterInstance* instance)
	:	m_instance(instance)
	{
	}

	bool operator () (const std::pair< int32_t, FlashDisplayList::Layer >& it) const
	{
		return it.second.instance == m_instance;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashDisplayList", FlashDisplayList, Object)

FlashDisplayList::FlashDisplayList(ActionContext* context)
:	m_context(context)
{
	reset();
}

void FlashDisplayList::reset()
{
	// Set default background color.
	m_backgroundColor.red =
	m_backgroundColor.green =
	m_backgroundColor.blue =
	m_backgroundColor.alpha = 255;

	// Clear all layers.
	m_layers.clear();
}

void FlashDisplayList::updateBegin(bool reset)
{
	if (reset)
	{
		for (layer_map_t::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
			i->second.collect = true;
	}
}

void FlashDisplayList::updateEnd()
{
	// Remove all layers which are still marked as "collect".
	for (layer_map_t::iterator i = m_layers.begin(); i != m_layers.end(); )
	{
		if (!i->second.immutable && i->second.collect)
			i = m_layers.erase(i);
		else
			i++;
	}
}

void FlashDisplayList::updateFrame(FlashCharacterInstance* ownerInstance, const FlashFrame* frame)
{
	// Update background color.
	if (frame->hasBackgroundColorChanged())
		m_backgroundColor = frame->getBackgroundColor();

	// Remove instances from active list.
	const SmallMap< uint16_t, FlashFrame::RemoveObject >& removeObjects = frame->getRemoveObjects();
	for (SmallMap< uint16_t, FlashFrame::RemoveObject >::const_iterator i = removeObjects.begin(); i != removeObjects.end(); ++i)
	{
		const FlashFrame::RemoveObject& removeObject = i->second;
		
		layer_map_t::iterator j = m_layers.find(removeObject.depth + c_depthOffset);
		if (j != m_layers.end())
		{
			if (removeObject.hasCharacterId)
			{
				if (j->second.id == removeObject.characterId)
					m_layers.erase(j);
			}
			else
				m_layers.erase(j);
		}
#if defined(_DEBUG)
		else
			log::warning << L"Unable to find character " << removeObject.characterId << L" in dictionary (1)" << Endl;
#endif
	}

	// Place instances onto active list.
	const SmallMap< uint16_t, FlashFrame::PlaceObject >& placeObjects = frame->getPlaceObjects();
	for (SmallMap< uint16_t, FlashFrame::PlaceObject >::const_iterator i = placeObjects.begin(); i != placeObjects.end(); ++i)
	{
		const FlashFrame::PlaceObject& placeObject = i->second;
		if (placeObject.has(FlashFrame::PfHasMove) || placeObject.has(FlashFrame::PfHasCharacterId))
		{
			int32_t depth = placeObject.depth + c_depthOffset;
			Layer& layer = m_layers[depth];

			if (placeObject.has(FlashFrame::PfHasCharacterId) && placeObject.characterId != layer.id)
			{
				Ref< const FlashCharacter > character = m_context->getDictionary()->getCharacter(placeObject.characterId);
				if (character)
				{
					// Use previous instance transform if we're replacing an instance.
					Matrix33 transform = layer.instance ? layer.instance->getTransform() : Matrix33::identity();

					// Create new instance.
					layer.id = placeObject.characterId;
					layer.instance = character->createInstance(
						m_context,
						ownerInstance,
						placeObject.has(FlashFrame::PfHasName) ? placeObject.name : "",
						0,
						&placeObject.events
					);
					T_ASSERT (layer.instance);
					layer.instance->setTransform(transform);
				}
#if defined(_DEBUG)
				else
					log::warning << L"Unable to find character " << placeObject.characterId << L" in dictionary (2)" << Endl;
#endif
			}

			if (!layer.instance)
				continue;

			if (placeObject.has(FlashFrame::PfHasName))
				layer.instance->setName(placeObject.name);

			if (placeObject.has(FlashFrame::PfHasCxTransform))
				layer.instance->setColorTransform(placeObject.cxTransform);

			if (placeObject.has(FlashFrame::PfHasMatrix))
				layer.instance->setTransform(placeObject.matrix);

			if (placeObject.has(FlashFrame::PfHasFilters))
				layer.instance->setFilter(placeObject.filter);

			if (placeObject.has(FlashFrame::PfHasClipDepth))
				layer.clipDepth = placeObject.clipDepth + c_depthOffset;

			layer.name = m_context->getString(layer.instance->getName());
			layer.immutable = false;
			layer.collect = false;
		}
		else
		{
			T_ASSERT (!placeObject.has(FlashFrame::PfHasName));
			T_ASSERT (!placeObject.has(FlashFrame::PfHasCxTransform));
			T_ASSERT (!placeObject.has(FlashFrame::PfHasMatrix));

			layer_map_t::iterator j = m_layers.find(placeObject.depth + c_depthOffset);
			if (j != m_layers.end())
				m_layers.erase(j);
		}
	}
}

void FlashDisplayList::showObject(int32_t depth, uint16_t characterId, FlashCharacterInstance* characterInstance, bool immutable)
{
	T_ASSERT (characterInstance);
	m_layers[depth].id = characterId;
	m_layers[depth].name = m_context->getString(characterInstance->getName());
	m_layers[depth].instance = characterInstance;
	m_layers[depth].immutable = immutable;
}

void FlashDisplayList::removeObject(FlashCharacterInstance* characterInstance)
{
	T_ASSERT (characterInstance);
	
	layer_map_t::iterator i = std::find_if(m_layers.begin(), m_layers.end(), FindCharacter(characterInstance));
	T_ASSERT (i != m_layers.end());

	m_layers.erase(i);
}

int32_t FlashDisplayList::getObjectDepth(const FlashCharacterInstance* characterInstance) const
{
	T_ASSERT (characterInstance);
	for (layer_map_t::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		if (i->second.instance == characterInstance)
			return i->first;
	}
	return 0;
}

int32_t FlashDisplayList::getNextHighestDepth() const
{
	int32_t depth = 0;
	for (layer_map_t::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		depth = max(depth, i->first + 1);
	return depth;
}

void FlashDisplayList::swap(int32_t depth1, int32_t depth2)
{
	Layer& layer1 = m_layers[depth1];
	Layer& layer2 = m_layers[depth2];
	std::swap(layer1, layer2);
}

void FlashDisplayList::getVisibleObjects(RefArray< FlashCharacterInstance >& outCharacterInstances) const
{
	T_ASSERT (outCharacterInstances.empty());
	for (FlashDisplayList::layer_map_t::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		T_ASSERT (i->second.instance);
		if (i->second.instance->isVisible())
			outCharacterInstances.push_back(i->second.instance);
	}
}

	}
}
