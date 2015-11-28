#ifndef traktor_json_JsonNode_H
#define traktor_json_JsonNode_H

#include "Core/Object.h"

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

class T_DLLCLASS JsonNode : public Object
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_json_JsonNode_H
