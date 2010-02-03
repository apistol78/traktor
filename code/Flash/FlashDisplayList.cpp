#include "Core/Log/Log.h"
#include "Flash/FlashDisplayList.h"
#include "Flash/FlashMovie.h"
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
		if (i->second.collect)
			m_layers.erase(i++);
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
	const std::map< uint16_t, FlashFrame::RemoveObject >& removeObjects = frame->getRemoveObjects();
	for (std::map< uint16_t, FlashFrame::RemoveObject >::const_iterator i = removeObjects.begin(); i != removeObjects.end(); ++i)
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
	}

	// Place instances onto active list.
	const std::map< uint16_t, FlashFrame::PlaceObject >& placeObjects = frame->getPlaceObjects();
	for (std::map< uint16_t, FlashFrame::PlaceObject >::const_iterator i = placeObjects.begin(); i != placeObjects.end(); ++i)
	{
		const FlashFrame::PlaceObject& placeObject = i->second;
		if (placeObject.hasMove || placeObject.hasCharacterId)
		{
			Layer& layer = m_layers[placeObject.depth + c_depthOffset];

			if (placeObject.hasCharacterId && placeObject.characterId != layer.id)
			{
				Ref< const FlashCharacter > character = m_context->getMovie()->getCharacter(placeObject.characterId);
				if (character)
				{
					// Use previous instance transform if we're replacing an instance.
					Matrix33 transform = layer.instance ? layer.instance->getTransform() : Matrix33::identity();

					layer.id = placeObject.characterId;
					layer.instance = character->createInstance(m_context, ownerInstance);
					layer.instance->setTransform(transform);
				}
				else
					log::warning << L"Unable to find character " << placeObject.characterId << L" in dictionary" << Endl;
			}

			if (!layer.instance)
				continue;

			if (placeObject.hasName)
				layer.instance->setName(placeObject.name);

			if (placeObject.hasCxTransform)
				layer.instance->setColorTransform(placeObject.cxTransform);

			if (placeObject.hasMatrix)
				layer.instance->setTransform(placeObject.matrix);

			if (!placeObject.actions.empty())
			{
				for (std::vector< FlashFrame::PlaceAction >::const_iterator j = placeObject.actions.begin(); j != placeObject.actions.end(); ++j)
					layer.instance->setEvent(j->eventMask, j->script);
			}

			if (placeObject.hasClipDepth)
				layer.clipDepth = placeObject.clipDepth + c_depthOffset;

			layer.collect = false;
		}
		else
		{
			T_ASSERT (!placeObject.hasName);
			T_ASSERT (!placeObject.hasCxTransform);
			T_ASSERT (!placeObject.hasMatrix);

			layer_map_t::iterator j = m_layers.find(placeObject.depth + c_depthOffset);
			if (j != m_layers.end())
				m_layers.erase(j);
		}
	}
}

void FlashDisplayList::showObject(int32_t depth, uint16_t characterId, FlashCharacterInstance* characterInstance)
{
	m_layers[depth].id = characterId;
	m_layers[depth].instance = characterInstance;
}

void FlashDisplayList::removeObject(FlashCharacterInstance* characterInstance)
{
	for (layer_map_t::iterator i = m_layers.begin(); i != m_layers.end(); )
	{
		if (i->second.instance == characterInstance)
			m_layers.erase(i++);
		else
			++i;
	}
}

int32_t FlashDisplayList::getObjectDepth(const FlashCharacterInstance* characterInstance) const
{
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

const SwfColor& FlashDisplayList::getBackgroundColor() const
{
	return m_backgroundColor;
}

const FlashDisplayList::layer_map_t& FlashDisplayList::getLayers() const
{
	return m_layers;
}

const FlashDisplayList::layer_map_t::const_iterator FlashDisplayList::findLayer(const std::wstring& name) const
{
	for (layer_map_t::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		if (i->second.instance && i->second.instance->getName() == name)
			return i;
	}
	return m_layers.end();
}

	}
}

