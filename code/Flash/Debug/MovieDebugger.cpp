/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/RefArray.h"
#include "Flash/FlashButtonInstance.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashMorphShapeInstance.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashShapeInstance.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashTextInstance.h"
#include "Flash/Debug/ButtonInstanceDebugInfo.h"
#include "Flash/Debug/CaptureControl.h"
#include "Flash/Debug/EditInstanceDebugInfo.h"
#include "Flash/Debug/MorphShapeInstanceDebugInfo.h"
#include "Flash/Debug/MovieDebugger.h"
#include "Flash/Debug/MovieDebugInfo.h"
#include "Flash/Debug/PostFrameDebugInfo.h"
#include "Flash/Debug/ShapeInstanceDebugInfo.h"
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
	if (const FlashButtonInstance* buttonInstance = dynamic_type_cast< const FlashButtonInstance* >(instance))
	{
		outDebugInfo.push_back(new ButtonInstanceDebugInfo(buttonInstance));
	}
	else if (const FlashEditInstance* editInstance = dynamic_type_cast< const FlashEditInstance* >(instance))
	{
		outDebugInfo.push_back(new EditInstanceDebugInfo(editInstance));
	}
	else if (const FlashMorphShapeInstance* morphShapeInstance = dynamic_type_cast< const FlashMorphShapeInstance* >(instance))
	{
		outDebugInfo.push_back(new MorphShapeInstanceDebugInfo(morphShapeInstance));
	}
	else if (const FlashShapeInstance* shapeInstance = dynamic_type_cast< const FlashShapeInstance* >(instance))
	{
		outDebugInfo.push_back(new ShapeInstanceDebugInfo(shapeInstance));
	}
	else if (const FlashSpriteInstance* spriteInstance = dynamic_type_cast< const FlashSpriteInstance* >(instance))
	{
		RefArray< InstanceDebugInfo > childrenDebugInfo;
		spriteInstance->getDisplayList().forEachVisibleObjectDirect([&](FlashCharacterInstance* child) {
			collectDebugInfo(child, childrenDebugInfo);
		});
		outDebugInfo.push_back(new SpriteInstanceDebugInfo(spriteInstance, childrenDebugInfo));
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
,	m_firstFrame(true)
,	m_captureFrames(0)
{
}

void MovieDebugger::postExecuteFrame(
	const FlashMovie* movie,
	const FlashSpriteInstance* movieInstance,
	const Vector4& stageTransform,
	int32_t viewWidth,
	int32_t viewHeight
) const
{
	Ref< CaptureControl > captureControl;
	if (m_transport->recv< CaptureControl >(0, captureControl) == net::BidirectionalObjectTransport::RtSuccess)
	{
		switch (captureControl->getMode())
		{
		case CaptureControl::MdSingle:
			m_captureFrames = 1;
			break;

		case CaptureControl::MdContinuous:
			m_captureFrames = std::numeric_limits< int32_t >::max();
			break;

		default:
			m_captureFrames = 0;
			break;
		}
	}

	if (m_firstFrame)
	{
		MovieDebugInfo movieInfo(L"unnamed", movie);
		m_transport->send(&movieInfo);
		m_firstFrame = false;
	}

	if (m_captureFrames > 0)
	{
		RefArray< InstanceDebugInfo > debugInfo;
		collectDebugInfo(movieInstance, debugInfo);
		PostFrameDebugInfo postFrameInfo(
			movie->getFrameBounds(),
			stageTransform,
			viewWidth,
			viewHeight,
			debugInfo
		);
		m_transport->send(&postFrameInfo);
		m_captureFrames--;
	}
}

	}
}
