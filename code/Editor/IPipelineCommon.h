#ifndef traktor_editor_IPipelineCommon_H
#define traktor_editor_IPipelineCommon_H

#include "Core/Guid.h"
#include "Core/Object.h"

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
class IStream;
class Path;

	namespace db
	{

class Database;

	}

	namespace editor
	{

/*! \brief Pipeline common interface.
 * \ingroup Editor
 */
class T_DLLCLASS IPipelineCommon : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< db::Database > getSourceDatabase() const = 0;

	virtual Ref< db::Database > getOutputDatabase() const = 0;

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) = 0;

	virtual Ref< IStream > openFile(const Path& basePath, const std::wstring& fileName) = 0;

	virtual Ref< IStream > createTemporaryFile(const std::wstring& fileName) = 0;

	virtual Ref< IStream > openTemporaryFile(const std::wstring& fileName) = 0;

	template < typename T >
	Ref< const T > getObjectReadOnly(const Guid& guid)
	{
		return dynamic_type_cast< const T* >(getObjectReadOnly(guid));
	}
};

	}
}

#endif	// traktor_editor_IPipelineCommon_H
