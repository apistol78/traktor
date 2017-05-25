/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_CaptureControl_H
#define traktor_flash_CaptureControl_H

#include "Core/Serialization/ISerializable.h"

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
	
class T_DLLCLASS CaptureControl : public ISerializable
{
	T_RTTI_CLASS;

public:
	CaptureControl();

	CaptureControl(int32_t frameCount);

	int32_t getFrameCount() const { return m_frameCount; }

	virtual void serialize(ISerializer& s);

private:
	int32_t m_frameCount;
};

	}
}

#endif	// traktor_flash_CaptureControl_H
