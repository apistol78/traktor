/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Flash/Dictionary.h"
#include "Flash/Frame.h"
#include "Flash/Sprite.h"
#include "Flash/SpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionValueArray.h"
#include "Flash/Action/IActionVMImage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.Sprite", 1, Sprite, Character)

Sprite::Sprite()
:	m_frameRate(0)
{
}

Sprite::Sprite(uint16_t id, uint16_t frameRate)
:	Character(id)
,	m_frameRate(frameRate)
{
}

uint16_t Sprite::getFrameRate() const
{
	return m_frameRate;
}

void Sprite::addFrame(Frame* frame)
{
	m_frames.push_back(frame);
}

uint32_t Sprite::getFrameCount() const
{
	return uint32_t(m_frames.size());
}

Frame* Sprite::getFrame(uint32_t frameId) const
{
	return frameId < m_frames.size() ? m_frames[frameId] : 0;
}

int Sprite::findFrame(const std::string& frameLabel) const
{
	for (RefArray< Frame >::const_iterator i = m_frames.begin(); i != m_frames.end(); ++i)
	{
		if (compareIgnoreCase< std::string >((*i)->getLabel(), frameLabel) == 0)
			return int(std::distance(m_frames.begin(), i));
	}
	return -1;
}

void Sprite::addInitActionScript(const IActionVMImage* initActionScript)
{
	m_initActionScripts.push_back(initActionScript);
}

const RefArray< const IActionVMImage >& Sprite::getInitActionScripts() const
{
	return m_initActionScripts;
}

void Sprite::setScalingGrid(const Aabb2& scalingGrid)
{
	m_scalingGrid = scalingGrid;
}

const Aabb2& Sprite::getScalingGrid() const
{
	return m_scalingGrid;
}

Ref< CharacterInstance > Sprite::createInstance(
	ActionContext* context,
	Dictionary* dictionary,
	CharacterInstance* parent,
	const std::string& name,
	const Matrix33& transform,
	const ActionObject* initObject,
	const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
) const
{
	Ref< SpriteInstance > spriteInstance = new SpriteInstance(context, dictionary, parent, this);

	if (!name.empty())
		spriteInstance->setName(name);

	if (events)
		spriteInstance->setEvents(*events);

	spriteInstance->setTransform(transform);

	if (initObject)
	{
		ActionObject* spriteInstanceAO = spriteInstance->getAsObject(context);
		T_ASSERT (spriteInstanceAO);

		const ActionObject::member_map_t& members = initObject->getLocalMembers();
		for (ActionObject::member_map_t::const_iterator i = members.begin(); i != members.end(); ++i)
		{
			// \fixme Same as enum2; need a visible member enumerator.
			if (i->first != ActionContext::Id__proto__ && i->first != ActionContext::IdPrototype)
				spriteInstanceAO->setMember(i->first, i->second);
		}
	}

	std::string spriteClassName;
	if (dictionary->getExportName(getId(), spriteClassName))
	{
		ActionValue spriteClassValue;
		if (context->getGlobal()->getMember(spriteClassName, spriteClassValue))
		{
			ActionObject* spriteInstanceAO = spriteInstance->getAsObject(context);
			T_ASSERT (spriteInstanceAO);

			ActionValue prototype;
			spriteClassValue.getObjectAlways(context)->getMember(ActionContext::IdPrototype, prototype);
			spriteInstanceAO->setMember(ActionContext::Id__proto__, prototype);
			spriteInstanceAO->setMember(ActionContext::Id__ctor__, spriteClassValue);
		}
	}

	spriteInstance->eventConstruct();
	spriteInstance->eventInit();

	spriteInstance->updateDisplayList();

	if (dictionary->getExportName(getId(), spriteClassName))
	{
		ActionValue spriteClassValue;
		if (context->getGlobal()->getMember(spriteClassName, spriteClassValue))
		{
			ActionObject* spriteInstanceAO = spriteInstance->getAsObject(context);
			T_ASSERT (spriteInstanceAO);

			ActionFunction* classConstructor = spriteClassValue.getObject< ActionFunction >();
			if (classConstructor)
			{
				Ref< SpriteInstance > current = context->getMovieClip();
				context->setMovieClip(spriteInstance);

				classConstructor->call(spriteInstanceAO);

				context->setMovieClip(current);
			}
		}
	}

	return spriteInstance;
}

void Sprite::serialize(ISerializer& s)
{
	Character::serialize(s);

	s >> Member< uint16_t >(L"frameRate", m_frameRate);
	s >> MemberRefArray< Frame >(L"frames", m_frames);
	s >> MemberRefArray< const IActionVMImage >(L"initActionScripts", m_initActionScripts);
	s >> MemberAabb2(L"scalingGrid", m_scalingGrid);
}

	}
}
