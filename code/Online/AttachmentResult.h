/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_AttachmentResult_H
#define traktor_online_AttachmentResult_H

#include "Online/Result.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace online
	{

class T_DLLCLASS AttachmentResult : public Result
{
	T_RTTI_CLASS;

public:
	void succeed(ISerializable* attachment);

	ISerializable* get() const;

private:
	Ref< ISerializable > m_attachment;
};

	}
}

#endif	// traktor_online_AttachmentResult_H
