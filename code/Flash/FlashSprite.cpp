#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"

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
		if (compareIgnoreCase((*i)->getLabel(), frameLabel) == 0)
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

Ref< FlashCharacterInstance > FlashSprite::createInstance(ActionContext* context, FlashCharacterInstance* parent) const
{
	Ref< FlashSpriteInstance > spriteInstance = new FlashSpriteInstance(context, parent, this);

	// See if MovieClip has another class registered.
	std::wstring exportName;
	if (context->getMovie()->getExportName(getId(), exportName))
	{
		ActionValue movieClipClass;
		if (context->getGlobal()->getLocalMember(exportName, movieClipClass))
		{
			ActionValue prototype;
			movieClipClass.getObject()->getLocalMember(L"prototype", prototype);
			spriteInstance->setMember(L"prototype", prototype);
			spriteInstance->setMember(L"__proto__", prototype);
		}
	}

	return spriteInstance;
}

void FlashSprite::trace(const IVisitor& visitor) const
{
	for (RefArray< FlashFrame >::const_iterator i = m_frames.begin(); i != m_frames.end(); ++i)
		visitor(*i);
	FlashCharacter::trace(visitor);
}

void FlashSprite::dereference()
{
	m_frames.clear();
	FlashCharacter::dereference();
}

	}
}
