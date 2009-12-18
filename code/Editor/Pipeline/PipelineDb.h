#ifndef traktor_editor_PipelineDb_H
#define traktor_editor_PipelineDb_H

#include "Editor/IPipelineDb.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sql
	{

class IConnection;

	}

	namespace editor
	{

class T_DLLCLASS PipelineDb : public IPipelineDb
{
	T_RTTI_CLASS;

public:
	bool open(const std::wstring& connectionString);

	void close();

	virtual void set(const Guid& guid, const Hash& hash);

	virtual bool get(const Guid& guid, Hash& outHash) const;

	virtual Ref< IPipelineReport > createReport(const std::wstring& name, const Guid& guid);

private:
	Ref< sql::IConnection > m_connection;
};

	}
}

#endif	// traktor_editor_PipelineDb_H
