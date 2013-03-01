#ifndef traktor_script_Types_H
#define traktor_script_Types_H

#include <list>
#include "Core/Guid.h"

namespace traktor
{
	namespace script
	{

struct SourceMapping
{
	Guid id;
	std::wstring name;
	int32_t line;

	SourceMapping()
	:	line(0)
	{
	}
};

typedef std::list< SourceMapping > source_map_t;

	}
}

#endif	// traktor_script_Types_H
