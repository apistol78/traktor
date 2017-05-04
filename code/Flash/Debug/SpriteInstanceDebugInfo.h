/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_SpriteInstanceDebugInfo_H
#define traktor_flash_SpriteInstanceDebugInfo_H

#include "Core/RefArray.h"
#include "Flash/Debug/InstanceDebugInfo.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class FlashSpriteInstance;
	
/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS SpriteInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	SpriteInstanceDebugInfo();

	SpriteInstanceDebugInfo(const FlashSpriteInstance* instance, const RefArray< InstanceDebugInfo >& childrenDebugInfo);

	uint16_t getFrames() const { return m_frames; }

	uint16_t getCurrentFrame() const { return m_currentFrame; }

	bool isPlaying() const { return m_playing; }

	const RefArray< InstanceDebugInfo >& getChildrenDebugInfo() const { return m_childrenDebugInfo; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	uint16_t m_frames;
	uint16_t m_currentFrame;
	bool m_playing;
	RefArray< InstanceDebugInfo > m_childrenDebugInfo;
};
	
	}
}

#endif	// traktor_flash_SpriteInstanceDebugInfo_H
