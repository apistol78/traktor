#ifndef traktor_json_JsonDocument_H
#define traktor_json_JsonDocument_H

#include "Core/Io/IStream.h"
#include "Core/Io/Path.h"
#include "Json/JsonArray.h"

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

/*! \brief
 * \ingroup JSON
 */
class T_DLLCLASS JsonDocument : public JsonArray
{
	T_RTTI_CLASS;

public:
	bool loadFromFile(const Path& fileName);
	
	bool loadFromStream(IStream* stream);
	
	bool loadFromText(const std::wstring& text);
};

	}
}

#endif	// traktor_json_JsonDocument_H
