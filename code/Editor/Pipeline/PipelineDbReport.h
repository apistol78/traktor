#ifndef traktor_editor_PipelineDbReport_H
#define traktor_editor_PipelineDbReport_H

#include <map>
#include "Core/Guid.h"
#include "Editor/IPipelineReport.h"

namespace traktor
{
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
	PipelineDbReport(sql::IConnection* connection, const std::wstring& table, const Guid& guid);

	virtual ~PipelineDbReport();

	virtual void set(const std::wstring& name, int32_t value);

private:
	Ref< sql::IConnection > m_connection;
	std::wstring m_table;
	Guid m_guid;
	std::map< std::wstring, int32_t > m_values;
};

	}
}

#endif	// traktor_editor_PipelineDbReport_H
