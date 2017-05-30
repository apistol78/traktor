/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_SpriteInstanceDebugInfo_H
#define traktor_flash_SpriteInstanceDebugInfo_H

#include <map>
#include <utility>
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
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

class SpriteInstance;
	
/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS SpriteInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	SpriteInstanceDebugInfo();

	SpriteInstanceDebugInfo(const SpriteInstance* instance, const std::string& className, bool mask, bool clipped, const RefArray< InstanceDebugInfo >& childrenDebugInfo);

	const std::string& getClassName() const { return m_className; }

	bool getMask() const { return m_mask; }

	bool getClipped() const { return m_clipped; }

	uint16_t getFrames() const { return m_frames; }

	uint16_t getCurrentFrame() const { return m_currentFrame; }

	bool isPlaying() const { return m_playing; }

	const Aabb2& getScalingGrid() const { return m_scalingGrid; }

	const AlignedVector< std::pair< uint32_t, std::string > >& getLabels() const { return m_labels; }

	const RefArray< InstanceDebugInfo >& getChildrenDebugInfo() const { return m_childrenDebugInfo; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::string m_className;
	bool m_mask;
	bool m_clipped;
	uint16_t m_frames;
	uint16_t m_currentFrame;
	bool m_playing;
	Aabb2 m_scalingGrid;
	AlignedVector< std::pair< uint32_t, std::string > > m_labels;
	RefArray< InstanceDebugInfo > m_childrenDebugInfo;
};
	
	}
}

#endif	// traktor_flash_SpriteInstanceDebugInfo_H
