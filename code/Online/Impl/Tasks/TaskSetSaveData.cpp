#include "Online/AttachmentResult.h"
#include "Online/Impl/Tasks/TaskSetSaveData.h"
#include "Online/Provider/ISaveDataProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskSetSaveData", TaskSetSaveData, ITask)

TaskSetSaveData::TaskSetSaveData(
	ISaveDataProvider* provider,
	const std::wstring& saveDataId,
	const ISerializable* attachment,
	bool replace,
	Result* result
)
:	m_provider(provider)
,	m_saveDataId(saveDataId)
,	m_attachment(attachment)
,	m_replace(replace)
,	m_result(result)
{
}

void TaskSetSaveData::execute()
{
	T_ASSERT (m_provider);
	T_ASSERT (m_result);
	if (m_provider->set(
		m_saveDataId,
		m_attachment,
		m_replace
	))
		m_result->succeed();
	else
		m_result->fail();
}

	}
}
