#pragma once

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

class T_DLLCLASS MovieDebugInfo : public ISerializable
{
	T_RTTI_CLASS;

public:
	MovieDebugInfo();

	MovieDebugInfo(const std::wstring& name);

	const std::wstring& getName() const { return m_name; }

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_name;
};

	}
}

