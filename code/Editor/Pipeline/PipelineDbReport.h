/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_PipelineDbReport_H
#define traktor_editor_PipelineDbReport_H

#include <map>
#include "Core/Guid.h"
#include "Editor/IPipelineReport.h"

namespace traktor
{

class ReaderWriterLock;

	namespace sql
	{

class IConnection;

	}

	namespace editor
	{

/*! \brief Pipeline db reporting.
 * \ingroup Editor.
 */
class PipelineDbReport : public IPipelineReport
{
	T_RTTI_CLASS;

public:
	PipelineDbReport(ReaderWriterLock& lock, sql::IConnection* connection, const std::wstring& table, const Guid& guid);

	virtual ~PipelineDbReport();

	virtual void set(const std::wstring& name, int32_t value) T_OVERRIDE T_FINAL;

	virtual void set(const std::wstring& name, const std::wstring& value) T_OVERRIDE T_FINAL;

private:
	ReaderWriterLock& m_lock;
	Ref< sql::IConnection > m_connection;
	std::wstring m_table;
	Guid m_guid;
	std::map< std::wstring, int32_t > m_ivalues;
	std::map< std::wstring, std::wstring > m_svalues;
};

	}
}

#endif	// traktor_editor_PipelineDbReport_H
