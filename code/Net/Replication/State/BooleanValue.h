/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_BooleanValue_H
#define traktor_net_BooleanValue_H

#include "Net/Replication/State/IValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class T_DLLCLASS BooleanValue : public RefCountImpl< IValue >
{
	T_RTTI_CLASS;

public:
	typedef bool value_t;
	typedef bool value_argument_t;
	typedef bool value_return_t;

	BooleanValue()
	:	m_value(false)
	{
	}

	BooleanValue(bool value)
	:	m_value(value)
	{
	}

	operator bool () const { return m_value; }

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	bool m_value;
};

	}
}

#endif	// traktor_net_BooleanValue_H
