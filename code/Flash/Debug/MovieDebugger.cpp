/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/RefArray.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashTextInstance.h"
#include "Flash/Debug/EditInstanceDebugInfo.h"
#include "Flash/Debug/MovieDebugger.h"
#include "Flash/Debug/PostFrameDebugInfo.h"
#include "Flash/Debug/SpriteInstanceDebugInfo.h"
#include "Flash/Debug/TextInstanceDebugInfo.h"
#include "Net/BidirectionalObjectTransport.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{
		
void collectDebugInfo(const FlashCharacterInstance* instance, RefArray< InstanceDebugInfo >& outDebugInfo)
{
	if (const FlashEditInstance* editInstance = dynamic_type_cast< const FlashEditInstance* >(instance))
	{
		outDebugInfo.push_back(new EditInstanceDebugInfo(editInstance));
	}
	else if (const FlashSpriteInstance* spriteInstance = dynamic_type_cast< const FlashSpriteInstance* >(instance))
	{
		outDebugInfo.push_back(new SpriteInstanceDebugInfo(spriteInstance));
		spriteInstance->getDisplayList().forEachVisibleObjectDirect([&](FlashCharacterInstance* child) {
			collectDebugInfo(child, outDebugInfo);
		});
	}
	else if (const FlashTextInstance* textInstance = dynamic_type_cast< const FlashTextInstance* >(instance))
	{
		outDebugInfo.push_back(new TextInstanceDebugInfo(textInstance));
	}
}
		
		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.MovieDebugger", MovieDebugger, Object)

MovieDebugger::MovieDebugger(net::BidirectionalObjectTransport* transport)
:	m_transport(transport)
{
	m_timer.start();
}

void MovieDebugger::postExecuteFrame(const FlashMovie* movie, const FlashSpriteInstance* movieInstance) const
{
	if (m_timer.getElapsedTime() >= 1.0)
	{
		RefArray< InstanceDebugInfo > debugInfo;
		collectDebugInfo(movieInstance, debugInfo);
		m_transport->send(new PostFrameDebugInfo(movie->getFrameBounds(), debugInfo));
		m_timer.start();
	}
}

	}
}
