/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_weather_CloudMaskResource_H
#define traktor_weather_CloudMaskResource_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace weather
	{

class T_DLLCLASS CloudMaskResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	CloudMaskResource(int32_t size = 0);

	int32_t getSize() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	int32_t m_size;
};

	}
}

#endif	// traktor_weather_CloudMaskResource_H
