#ifndef traktor_json_JsonObject_H
#define traktor_json_JsonObject_H

#include "Core/RefArray.h"
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

class JsonMember;

class T_DLLCLASS JsonObject : public JsonNode
{
	T_RTTI_CLASS;

public:
	JsonObject();

	void push(JsonMember* member);

	bool empty() const { return m_members.empty(); }

	JsonMember* front() { return m_members.front(); }

	JsonMember* back() { return m_members.back(); }

	uint32_t size() const { return uint32_t(m_members.size()); }

	JsonMember* getMember(const std::wstring& name);

	Any getMemberValue(const std::wstring& name);

	JsonMember* get(uint32_t index) { return m_members[index]; }

	const RefArray< JsonMember >& get() const { return m_members; }

private:
	RefArray< JsonMember > m_members;
};

	}
}

#endif	// traktor_json_JsonObject_H
