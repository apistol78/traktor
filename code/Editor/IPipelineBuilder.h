#ifndef traktor_editor_IPipelineBuilder_H
#define traktor_editor_IPipelineBuilder_H

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace db
	{

class Database;
class Instance;

	}

	namespace editor
	{

class IPipelineReport;
class PipelineDependency;

class T_DLLCLASS IPipelineBuilder : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool build(const RefArray< PipelineDependency >& dependencies, bool rebuild) = 0;

	virtual bool buildOutput(const ISerializable* sourceAsset, const Object* buildParams, const std::wstring& name, const std::wstring& outputPath, const Guid& outputGuid) = 0;

	virtual Ref< db::Database > getSourceDatabase() const = 0;

	virtual Ref< db::Database > getOutputDatabase() const = 0;

	virtual Ref< db::Instance > createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid) = 0;

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) = 0;

	virtual Ref< IPipelineReport > createReport(const std::wstring& name, const Guid& guid) = 0;

	template < typename T >
	Ref< const T > getObjectReadOnly(const Guid& guid)
	{
		return dynamic_type_cast< const T* >(getObjectReadOnly(guid));
	}
};

	}
}

#endif	// traktor_editor_IPipelineBuilder_H
