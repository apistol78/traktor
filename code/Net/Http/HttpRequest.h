/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_HttpRequest_H
#define traktor_net_HttpRequest_H

#include <string>
#include <map>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class T_DLLCLASS HttpRequest : public Object
{
	T_RTTI_CLASS;

public:
	enum Method
	{
		MtUnknown,
		MtGet,
		MtHead,
		MtPost,
		MtPut,
		MtDelete,
		MtTrace,
		MtOptions,
		MtConnect,
		MtPatch
	};

	HttpRequest();

	Method getMethod() const;

	const std::wstring& getResource() const;

	bool hasValue(const std::wstring& key) const;

	void setValue(const std::wstring& key, const std::wstring& value);

	std::wstring getValue(const std::wstring& key) const;

	static Ref< HttpRequest > parse(const std::wstring& request);

private:
	Method m_method;
	std::wstring m_resource;
	std::map< std::wstring, std::wstring > m_values;
};

	}
}

#endif	// traktor_net_HttpRequest_H
