/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/RefArray.h"
#include "Flash/FlashButtonInstance.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashMorphShapeInstance.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashShapeInstance.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashTextInstance.h"
#include "Flash/Debug/ButtonInstanceDebugInfo.h"
#include "Flash/Debug/CaptureControl.h"
#include "Flash/Debug/EditInstanceDebugInfo.h"
#include "Flash/Debug/FrameDebugInfo.h"
#include "Flash/Debug/MorphShapeInstanceDebugInfo.h"
#include "Flash/Debug/MovieDebugger.h"
#include "Flash/Debug/MovieDebugInfo.h"
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
		
void collectDebugInfo(const FlashCharacterInstance* instance, bool mask, RefArray< InstanceDebugInfo >& outDebugInfo)
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
		outDebugInfo.push_back(new MorphShapeInstanceDebugInfo(morphShapeInstance, mask));
	}
	else if (const FlashShapeInstance* shapeInstance = dynamic_type_cast< const FlashShapeInstance* >(instance))
	{
		outDebugInfo.push_back(new ShapeInstanceDebugInfo(shapeInstance, mask));
	}
	else if (const FlashSpriteInstance* spriteInstance = dynamic_type_cast< const FlashSpriteInstance* >(instance))
	{
		std::string className;
		spriteInstance->getDictionary()->getExportName(spriteInstance->getSprite()->getId(), className);

		RefArray< InstanceDebugInfo > childrenDebugInfo;

		const FlashDisplayList::layer_map_t& layers = spriteInstance->getDisplayList().getLayers();
		for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); )
		{
			const FlashDisplayList::Layer& layer = i->second;
			if (!layer.instance)
			{
				++i;
				continue;
			}

			if (!layer.clipEnable)
			{
				collectDebugInfo(layer.instance, false, childrenDebugInfo);
				++i;
			}
			else
			{
				collectDebugInfo(layer.instance, true, childrenDebugInfo);
				for (++i; i != layers.end(); ++i)
				{
					if (i->first > layer.clipDepth)
						break;

					const FlashDisplayList::Layer& clippedLayer = i->second;
					if (!clippedLayer.instance)
						continue;

					collectDebugInfo(clippedLayer.instance, false, childrenDebugInfo);
				}
			}
		}

		outDebugInfo.push_back(new SpriteInstanceDebugInfo(spriteInstance, className, childrenDebugInfo));
	}
	else if (const FlashTextInstance* textInstance = dynamic_type_cast< const FlashTextInstance* >(instance))
	{
		outDebugInfo.push_back(new TextInstanceDebugInfo(textInstance));
	}
}
		
		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.MovieDebugger", MovieDebugger, Object)

MovieDebugger::MovieDebugger(net::BidirectionalObjectTransport* transport, const std::wstring& name)
:	m_transport(transport)
,	m_captureFrames(0)
{
	MovieDebugInfo movieInfo(name);
	m_transport->send(&movieInfo);

	Ref< CaptureControl > captureControl;
	if (m_transport->recv< CaptureControl >(100, captureControl) == net::BidirectionalObjectTransport::RtSuccess)
		m_captureFrames = captureControl->getFrameCount();
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
		m_captureFrames = captureControl->getFrameCount();

	if (m_captureFrames > 0)
	{
		RefArray< InstanceDebugInfo > debugInfo;
		collectDebugInfo(movieInstance, false, debugInfo);
		FrameDebugInfo postFrameInfo(
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
