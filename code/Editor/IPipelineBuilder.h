#ifndef traktor_editor_IPipelineBuilder_H
#define traktor_editor_IPipelineBuilder_H

#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Heap/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Serializable;

	namespace db
	{

class Database;
class Instance;

	}

	namespace editor
	{

class PipelineDependency;

class T_DLLCLASS IPipelineBuilder : public Object
{
	T_RTTI_CLASS(IPipelineBuilder)

public:
	virtual bool build(const RefArray< PipelineDependency >& dependencies, bool rebuild) = 0;

	virtual db::Database* getSourceDatabase() const = 0;

	virtual db::Database* getOutputDatabase() const = 0;

	virtual db::Instance* createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid) = 0;

	virtual const Serializable* getObjectReadOnly(const Guid& instanceGuid) = 0;

	template < typename T >
	const T* getObjectReadOnly(const Guid& guid)
	{
		return dynamic_type_cast< const T* >(getObjectReadOnly(guid));
	}
};

	}
}

#endif	// traktor_editor_IPipelineBuilder_H
