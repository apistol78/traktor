#include "Online/Impl/Tasks/TaskEnumSaveData.h"
#include "Online/Provider/ISaveDataProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskEnumSaveData", TaskEnumSaveData, ITask)

TaskEnumSaveData::TaskEnumSaveData(
	ISaveDataProvider* provider,
	Object* sinkObject,
	sink_method_t sinkMethod
)
:	m_provider(provider)
,	m_sinkObject(sinkObject)
,	m_sinkMethod(sinkMethod)
{
}

void TaskEnumSaveData::execute()
{
	T_ASSERT (m_provider);
	std::set< std::wstring > saveDataIds;
	m_provider->enumerate(saveDataIds);
	(m_sinkObject->*m_sinkMethod)(saveDataIds);
}

	}
}
