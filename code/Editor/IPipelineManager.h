#ifndef traktor_editor_IPipelineManager_H
#define traktor_editor_IPipelineManager_H

#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Io/Path.h"
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

class IPipeline;
class IPipelineCache;
class PipelineDependency;

class T_DLLCLASS IPipelineManager : public Object
{
	T_RTTI_CLASS(IPipelineManager)

public:
	virtual IPipeline* findPipeline(const Type& sourceType) const = 0;

	virtual void addDependency(
		const Serializable* sourceAsset
	) = 0;

	virtual void addDependency(
		const Serializable* sourceAsset,
		const std::wstring& name,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		bool build
	) = 0;

	virtual void addDependency(
		db::Instance* sourceAssetInstance,
		bool build
	) = 0;

	virtual void addDependency(
		const Guid& sourceAssetGuid,
		bool build
	) = 0;

	virtual void addDependency(
		const Path& fileName
	) = 0;

	virtual void getDependencies(RefArray< PipelineDependency >& outDependencies) const = 0;

	virtual bool build(bool rebuild) = 0;

	virtual db::Database* getSourceDatabase() const = 0;

	virtual db::Database* getOutputDatabase() const = 0;

	virtual IPipelineCache* getCache() const = 0;

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

#endif	// traktor_editor_IPipelineManager_H
