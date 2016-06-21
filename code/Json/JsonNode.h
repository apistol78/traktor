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

class OutputStream;

	namespace json
	{

/*! \brief JSON abstract node.
 * \ingroup JSON
 */
class T_DLLCLASS JsonNode : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Write JSON respresentation of node into stream.
	 *
	 * \param os Output stream.
	 * \return True if successfully written.
	 */
	virtual bool write(OutputStream& os) const = 0;
};

	}
}

#endif	// traktor_json_JsonNode_H
