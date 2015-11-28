#ifndef traktor_json_JsonMember_H
#define traktor_json_JsonMember_H

#include "Core/Class/Any.h"
#include "Json/JsonNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_JSON_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace json
	{

class T_DLLCLASS JsonMember : public JsonNode
{
	T_RTTI_CLASS;

public:
	JsonMember();

	JsonMember(const std::wstring& name, const Any& value);

	const std::wstring& getName() const { return m_name; }

	const Any& getValue() const { return m_value; }

private:
	std::wstring m_name;
	Any m_value;
};

	}
}

#endif	// traktor_json_JsonMember_H
