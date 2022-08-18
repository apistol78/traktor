#include "Core/RefArray.h"
#include "Spark/ButtonInstance.h"
#include "Spark/Dictionary.h"
#include "Spark/EditInstance.h"
#include "Spark/MorphShapeInstance.h"
#include "Spark/Movie.h"
#include "Spark/ShapeInstance.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"
#include "Spark/TextInstance.h"
#include "Spark/Debug/ButtonInstanceDebugInfo.h"
#include "Spark/Debug/CaptureControl.h"
#include "Spark/Debug/EditInstanceDebugInfo.h"
#include "Spark/Debug/FrameDebugInfo.h"
#include "Spark/Debug/MorphShapeInstanceDebugInfo.h"
#include "Spark/Debug/MovieDebugger.h"
#include "Spark/Debug/MovieDebugInfo.h"
#include "Spark/Debug/ShapeInstanceDebugInfo.h"
#include "Spark/Debug/SpriteInstanceDebugInfo.h"
#include "Spark/Debug/TextInstanceDebugInfo.h"
#include "Net/BidirectionalObjectTransport.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

void collectDebugInfo(const CharacterInstance* instance, bool mask, bool clipped, RefArray< InstanceDebugInfo >& outDebugInfo)
{
	if (const ButtonInstance* buttonInstance = dynamic_type_cast< const ButtonInstance* >(instance))
	{
		outDebugInfo.push_back(new ButtonInstanceDebugInfo(buttonInstance));
	}
	else if (const EditInstance* editInstance = dynamic_type_cast< const EditInstance* >(instance))
	{
		outDebugInfo.push_back(new EditInstanceDebugInfo(editInstance));
	}
	else if (const MorphShapeInstance* morphShapeInstance = dynamic_type_cast< const MorphShapeInstance* >(instance))
	{
		outDebugInfo.push_back(new MorphShapeInstanceDebugInfo(morphShapeInstance, mask, clipped));
	}
	else if (const ShapeInstance* shapeInstance = dynamic_type_cast< const ShapeInstance* >(instance))
	{
		outDebugInfo.push_back(new ShapeInstanceDebugInfo(shapeInstance, mask, clipped));
	}
	else if (const SpriteInstance* spriteInstance = dynamic_type_cast< const SpriteInstance* >(instance))
	{
		std::string className;
		//spriteInstance->getDictionary()->getExportName(spriteInstance->getSprite()->getId(), className);

		RefArray< InstanceDebugInfo > childrenDebugInfo;

		const DisplayList::layer_map_t& layers = spriteInstance->getDisplayList().getLayers();
		for (DisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); )
		{
			const DisplayList::Layer& layer = i->second;
			if (!layer.instance)
			{
				++i;
				continue;
			}

			if (!layer.clipEnable)
			{
				collectDebugInfo(layer.instance, false, clipped, childrenDebugInfo);
				++i;
			}
			else
			{
				collectDebugInfo(layer.instance, true, clipped, childrenDebugInfo);
				for (++i; i != layers.end(); ++i)
				{
					if (i->first > layer.clipDepth)
						break;

					const DisplayList::Layer& clippedLayer = i->second;
					if (!clippedLayer.instance)
						continue;

					collectDebugInfo(clippedLayer.instance, false, true, childrenDebugInfo);
				}
			}
		}

		outDebugInfo.push_back(new SpriteInstanceDebugInfo(spriteInstance, className, mask, clipped, childrenDebugInfo));
	}
	else if (const TextInstance* textInstance = dynamic_type_cast< const TextInstance* >(instance))
	{
		outDebugInfo.push_back(new TextInstanceDebugInfo(textInstance));
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.MovieDebugger", MovieDebugger, Object)

MovieDebugger::MovieDebugger(net::BidirectionalObjectTransport* transport, const std::wstring& name)
:	m_transport(transport)
,	m_captureFrames(0)
{
	MovieDebugInfo movieInfo(name);
	m_transport->send(&movieInfo);

	Ref< CaptureControl > captureControl;
	if (m_transport->recv< CaptureControl >(100, captureControl) == net::BidirectionalObjectTransport::Result::Success)
		m_captureFrames = captureControl->getFrameCount();
}

void MovieDebugger::postExecuteFrame(
	const Movie* movie,
	const SpriteInstance* movieInstance,
	const Vector4& stageTransform,
	int32_t viewWidth,
	int32_t viewHeight
) const
{
	Ref< CaptureControl > captureControl;
	if (m_transport->recv< CaptureControl >(0, captureControl) == net::BidirectionalObjectTransport::Result::Success)
		m_captureFrames = captureControl->getFrameCount();

	if (m_captureFrames > 0)
	{
		RefArray< InstanceDebugInfo > debugInfo;
		collectDebugInfo(movieInstance, false, false, debugInfo);
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
