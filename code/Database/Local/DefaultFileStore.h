#pragma once

#include "Database/Local/IFileStore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_LOCAL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! Default file store.
 * \ingroup Database
 */
class T_DLLCLASS DefaultFileStore : public IFileStore
{
	T_RTTI_CLASS;

public:
	DefaultFileStore();

	virtual bool create(const ConnectionString& connectionString) override final;

	virtual void destroy() override final;

	virtual uint32_t flags(const Path& filePath) override final;

	virtual bool add(const Path& filePath) override final;

	virtual bool remove(const Path& filePath) override final;

	virtual bool edit(const Path& filePath) override final;

	virtual bool rollback(const Path& filePath) override final;

	virtual bool clean(const Path& filePath) override final;

private:
	bool m_editReadOnly;
};

	}
}

