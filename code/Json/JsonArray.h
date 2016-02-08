#ifndef traktor_json_JsonArray_H
#define traktor_json_JsonArray_H

#include "Core/Class/Any.h"
#include "Core/Containers/AlignedVector.h"
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

class T_DLLCLASS JsonArray : public JsonNode
{
	T_RTTI_CLASS;

public:
	JsonArray();

	void push(const Any& value);

	bool empty() const { return m_array.empty(); }

	const Any& front() { return m_array.front(); }

	const Any& back() { return m_array.back(); }

	uint32_t size() const { return uint32_t(m_array.size()); }

	const Any& get(uint32_t index) { return m_array[index]; }

	const AlignedVector< Any >& get() const { return m_array; }

	virtual bool write(OutputStream& os) const T_OVERRIDE;

private:
	AlignedVector< Any > m_array;
};

	}
}

#endif	// traktor_json_JsonArray_H
