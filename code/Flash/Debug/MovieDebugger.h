/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_MovieDebugger_H
#define traktor_flash_MovieDebugger_H

#include "Core/Object.h"
#include "Core/Timer/Timer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{
	
class BidirectionalObjectTransport;

	}

	namespace flash
	{
	
class FlashMovie;
class FlashSpriteInstance;

/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS MovieDebugger : public Object
{
	T_RTTI_CLASS;

public:
	MovieDebugger(net::BidirectionalObjectTransport* transport);

	void postExecuteFrame(
		const FlashMovie* movie,
		const FlashSpriteInstance* movieInstance,
		const Vector4& stageTransform,
		int32_t viewWidth,
		int32_t viewHeight
	) const;

private:
	Ref< net::BidirectionalObjectTransport > m_transport;
	mutable bool m_firstFrame;
	mutable int32_t m_captureFrames;
};
	
	}
}

#endif	// traktor_flash_MovieDebugger_H
