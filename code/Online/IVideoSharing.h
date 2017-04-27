/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_IVideoSharing_H
#define traktor_online_IVideoSharing_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

class PropertyGroup;

	namespace online
	{

class T_DLLCLASS IVideoSharing : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool beginCapture(int32_t duration) = 0;

	virtual void endCapture(const PropertyGroup* metaData) = 0;

	virtual bool isCapturing() const = 0;

	virtual bool showShareUI() = 0;
};

	}
}

#endif	// traktor_online_IVideoSharing_H
