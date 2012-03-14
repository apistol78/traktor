#ifndef traktor_RfmEnum_H
#define traktor_RfmEnum_H

#include "Core/Reflection/ReflectionMember.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class T_DLLCLASS RfmEnum : public ReflectionMember
{
	T_RTTI_CLASS;

public:
	RfmEnum(const wchar_t* name, const std::wstring& value);

	void set(const std::wstring& value) { m_value = value; }

	const std::wstring& get() const { return m_value; }

private:
	std::wstring m_value;
};

}

#endif	// traktor_RfmEnum_H
