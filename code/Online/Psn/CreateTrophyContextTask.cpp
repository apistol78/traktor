#include <sysutil/sysutil_gamecontent.h>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Online/Psn/LogError.h"
#include "Online/Psn/CreateTrophyContextTask.h"

namespace traktor
{
	namespace online
	{
		namespace
		{


		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.CreateTrophyContextTask", CreateTrophyContextTask, ISaveGameQueueTask)

Ref< ISaveGameQueueTask > CreateTrophyContextTask::create(SceNpTrophyContext trophyContext, SceNpTrophyHandle trophyHandle)
{
	Ref< CreateTrophyContextTask > task = new CreateTrophyContextTask();
	task->m_trophyContext = trophyContext;
	task->m_trophyHandle = trophyHandle;
	return task;
}

bool CreateTrophyContextTask::execute()
{

	int32_t err = sceNpTrophyRegisterContext(m_trophyContext, m_trophyHandle, &callbackTrophyStatus, NULL, 0);

	if (err != 0)
	{
		LogError::logTrophyError(err);
		return false;
	}
	return true;
}

int CreateTrophyContextTask::callbackTrophyStatus(SceNpTrophyContext context, SceNpTrophyStatus status, int completed, int total, void *arg)
{
	switch (status)
	{

	case SCE_NP_TROPHY_STATUS_NOT_INSTALLED:
		log::error << L"Trophy configuration data is not installed." << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_DATA_CORRUPT:
		log::error << L"Trophy configuration data is corrupted." << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_INSTALLED:
		log::error << L"Trophy configuration data has been installed." << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_REQUIRES_UPDATE:
		log::error << L"A new trophy pack file is available in a patch." << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_CHANGES_DETECTED:
		log::error << L"Trophy configuration data was modified."  << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_UNKNOWN:
		log::error << L"Unknown status, internal error" << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_PROCESSING_SETUP:
		log::error << L"Processing setup" << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_PROCESSING_PROGRESS:
		log::error << L"Processing" << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_PROCESSING_FINALIZE:
		log::error << L"Processing termination processing" << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_PROCESSING_COMPLETE:
		log::error << L"Processing completed" << Endl;
		break;
	default:
		break;
	}
	log::error << L"Completed: " << completed << " Total: " << total << Endl;
}

	}
}
