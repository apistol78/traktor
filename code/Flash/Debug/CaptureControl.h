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
	enum Mode
	{
		MdStop,
		MdSingle,
		MdContinuous
	};

	CaptureControl();

	CaptureControl(Mode mode);

	Mode getMode() const { return m_mode; }

	virtual void serialize(ISerializer& s);

private:
	Mode m_mode;
};

	}
}

#endif	// traktor_flash_CaptureControl_H
