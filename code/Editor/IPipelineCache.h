#ifndef traktor_editor_IPipelineCache_H
#define traktor_editor_IPipelineCache_H

#include "Core/Object.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Stream;

	namespace editor
	{

class Settings;

class T_DLLCLASS IPipelineCache : public Object
{
	T_RTTI_CLASS(IPipelineCache)

public:
	virtual bool create(const Settings* settings) = 0;

	virtual Stream* get(const Guid& guid, uint32_t hash) const = 0;

	virtual Stream* put(const Guid& guid, uint32_t hash, Stream* source) = 0;
};

	}
}

#endif	// traktor_editor_IPipelineCache_H
