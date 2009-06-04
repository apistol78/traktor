#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashMovie.h"
#include "Flash/Action/ActionContext.h"
#include "Core/Heap/New.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashSprite", FlashSprite, FlashCharacter)

FlashSprite::FlashSprite(uint16_t id, uint16_t frameRate)
:	FlashCharacter(id)
,	m_frameRate(frameRate)
{
}

uint16_t FlashSprite::getFrameRate() const
{
	return m_frameRate;
}

void FlashSprite::addFrame(FlashFrame* frame)
{
	m_frames.push_back(frame);
}

uint32_t FlashSprite::getFrameCount() const
{
	return uint32_t(m_frames.size());
}

FlashFrame* FlashSprite::getFrame(uint32_t frameId) const
{
	return m_frames[frameId];
}

int FlashSprite::findFrame(const std::wstring& frameLabel) const
{
	for (RefArray< FlashFrame >::const_iterator i = m_frames.begin(); i != m_frames.end(); ++i)
	{
		if ((*i)->getLabel() == frameLabel)
			return int(std::distance(m_frames.begin(), i));
	}
	return -1;
}

void FlashSprite::addInitActionScript(ActionScript* initActionScript)
{
	m_initActionScripts.push_back(initActionScript);
}

const RefArray< ActionScript >& FlashSprite::getInitActionScripts() const
{
	return m_initActionScripts;
}

FlashCharacterInstance* FlashSprite::createInstance(ActionContext* context, FlashCharacterInstance* parent) const
{
	Ref< ActionObject > global = context->getGlobal();

	Ref< ActionContext > spriteContext = gc_new< ActionContext >(context->getMovie(), global);
	Ref< FlashSpriteInstance > spriteInstance = gc_new< FlashSpriteInstance >(spriteContext, parent, this);

	if (!parent)
	{
		global->setMember(L"_root", ActionValue::fromObject(spriteInstance));
		global->setMember(L"_level0", ActionValue::fromObject(spriteInstance));
	}

	// See if MovieClip has another class registered.
	std::wstring exportName;
	if (context->getMovie()->getExportName(getId(), exportName))
	{
		ActionValue movieClipClass;
		if (global->getMember(exportName, movieClipClass))
			spriteInstance->setMember(L"prototype", movieClipClass);
	}

	return spriteInstance;
}

	}
}
