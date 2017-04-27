/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_weather_CloudMask_H
#define traktor_weather_CloudMask_H

#include "Core/Object.h"
#include "Core/Misc/AutoPtr.h"

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

class T_DLLCLASS CloudMask : public Object
{
	T_RTTI_CLASS;

public:
	struct Sample
	{
		Sample()
		:	opacity(255)
		,	size(255)
		{
		}

		uint8_t opacity;
		uint8_t size;
	};

	CloudMask(int32_t size);

	int32_t getSize() const;

	Sample getSample(int32_t x, int32_t y) const;

private:
	friend class CloudMaskFactory;

	int32_t m_size;
	AutoArrayPtr< Sample > m_data;
};

	}
}

#endif	// traktor_weather_CloudMask_H
