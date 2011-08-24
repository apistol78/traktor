#ifndef traktor_editor_IPipelineDepends_H
#define traktor_editor_IPipelineDepends_H

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Io/Path.h"
#include "Editor/PipelineTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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

class IPipeline;
class PipelineDependency;

/*! \brief Pipeline dependency interface.
 * \ingroup Editor
 */
class T_DLLCLASS IPipelineDepends : public Object
{
	T_RTTI_CLASS;

public:
	virtual void addDependency(
		const ISerializable* sourceAsset
	) = 0;

	virtual void addDependency(
		const ISerializable* sourceAsset,
		const std::wstring& name,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t flags
	) = 0;

	virtual void addDependency(
		db::Instance* sourceAssetInstance,
		uint32_t flags
	) = 0;

	virtual void addDependency(
		const Guid& sourceAssetGuid,
		uint32_t flags
	) = 0;

	virtual void addDependency(
		const Path& fileName
	) = 0;

	virtual bool waitUntilFinished() = 0;

	virtual void getDependencies(RefArray< PipelineDependency >& outDependencies) const = 0;

	virtual Ref< db::Database > getSourceDatabase() const = 0;

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) = 0;

	template < typename T >
	Ref< const T > getObjectReadOnly(const Guid& guid)
	{
		return dynamic_type_cast< const T* >(getObjectReadOnly(guid));
	}
};

	}
}

#endif	// traktor_editor_IPipelineDepends_H
