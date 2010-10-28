#include <np.h>
#include <sysutil/sysutil_savedata.h>
#include <sysutil/sysutil_gamecontent.h>
#include "Core/Log/Log.h"
#include "Online/Psn/LogError.h"

namespace traktor
{
	namespace online
	{
		namespace
		{
		}

void LogError::logErrorSaveData(int32_t err)
{
	switch (err)
	{
	case CELL_SAVEDATA_ERROR_CBRESULT:
		log::error << L"Callback function returned an error" << Endl;
		break;
	case CELL_SAVEDATA_ERROR_ACCESS_ERROR:
		log::error << L"HDD access error" << Endl;
		break;
	case CELL_SAVEDATA_ERROR_INTERNAL:
		log::error << L"Fatal internal error" << Endl;
		break;
	case CELL_SAVEDATA_ERROR_PARAM:
		log::error << L"Error in parameter to be set to utility (application bug)" << Endl;
		break;
	case CELL_SAVEDATA_ERROR_NOSPACE:
		log::error << L"Insufficient free space (application bug: lack of free space must be judged and handled within the callback function)" << Endl;
		break;
	case CELL_SAVEDATA_ERROR_BROKEN:
		log::error << L"Save data corrupted (modification detected, etc.)" << Endl;
		break;
	case CELL_SAVEDATA_ERROR_FAILURE:
		log::error << L"Save/load of save data failed (file could not be found, etc.)" << Endl;
		break;
	case CELL_SAVEDATA_ERROR_BUSY:
		log::error << L"Save data utility function was called simultaneously" << Endl;
	default:
		log::error << L"Undefined error during save" << Endl;
	}
}

void LogError::logTrophyError(int32_t err)
{
	switch (err)
	{
	case SCE_NP_TROPHY_ERROR_NOT_INITIALIZED:
		log::error << L"Not initialized. sceNpTrophyInit() has not been called, or sceNpTrophyTerm()has already been called. Check the calling order." << Endl;
		break;
	case SCE_NP_TROPHY_ERROR_INVALID_ARGUMENT:
		log::error << L"Invalid argument. NULL was specified to statusCb. Check the argument value." << Endl;
		break;
	case SCE_NP_TROPHY_ERROR_UNKNOWN_CONTEXT:
		log::error << L"Unknown context. The specified context does not exist or it has already been destroyed. Check context." << Endl;
		break;
	case SCE_NP_TROPHY_ERROR_PROCESSING_ABORTED:
		log::error << L"An invalid value was returned from SceNpTrophyStatusCallback and further processing has been aborted." << Endl;
		break;
	case SCE_NP_TROPHY_ERROR_ABORT:
		log::error << L"The handle has been aborted. The processing has been aborted by sceNpTrophyAbortHandle(). Destroy handle. " << Endl;
		break;
	case SCE_NP_TROPHY_ERROR_UNKNOWN_HANDLE:
		log::error << L"Unknown handle. The specified handle does not exist or it has already been destroyed. Check handle." << Endl;
		break;
	case SCE_NP_TROPHY_ERROR_INVALID_NP_COMM_ID:
		log::error << L"Invalid NP Communication ID. Either the NP Communication ID or the NP Communication Signature is invalid. Check the argument values for when the context was created." << Endl;
		break;
	case SCE_NP_TROPHY_ERROR_UNKNOWN_NP_COMM_ID:
		log::error << L"NP Communication IDs do not match. The specified NP Communication ID does not match the NP Communication ID in the trophy pack file. Check the argument values for when the context was created." << Endl;
		break;
	case SCE_NP_TROPHY_ERROR_CONF_DOES_NOT_EXIST:
		log::error << L"Trophy pack file does not exist" << Endl;
		break;
	case SCE_NP_TROPHY_ERROR_VERIFICATION_FAILURE:
		log::error << L"Trophy pack file signature verification failed. The trophy pack file could not be properly verified. Be sure to access the Trophy Authoring Tool and append a digital signature to the trophy pack file." << Endl;
		break;
	case SCE_NP_TROPHY_ERROR_SHUTDOWN:
		log::error << L"The system is being shut down. The processing cannot be carried on and is therefore being canceled." << Endl;
		break;
	case SCE_NP_TROPHY_ERROR_TITLE_ICON_NOT_FOUND:
		log::error << L"Trophy set still image icon does not exist. Check that the trophy pack file contains the trophy set still image icon." << Endl;
		break;
	case SCE_NP_TROPHY_ERROR_TROPHY_ICON_NOT_FOUND:
		log::error << L"Trophy still image icon icon does not exist.Check that the trophy pack file contains all the trophy still image icons." << Endl;
		break;
	case SCE_NP_TROPHY_ERROR_INSUFFICIENT_DISK_SPACE:
		log::error << L"Insufficient disk space. There is not enough disk space to register the trophy set to the internal HDD. Notify the user accordingly. " << Endl;
		break;
	case SCE_NP_TROPHY_ERROR_ILLEGAL_UPDATE:
		log::error << L"Cannot update. An irresolvable discrepancy has been found between the registered data and the data trying to be registered.  Possible causes are from a change in trophy grade or a reduction in the number of trophies from the previous install. Check the trophy configuration data, and remove the previously installed trophy set from the console." << Endl;
		break;
	case SCE_NP_AUTH_EINVAL:
		log::error << L"Timing of the function call is invalid" << Endl;
		break;
	case SCE_NP_AUTH_ENOMEM:
		log::error << L"Insufficient memory - memory could not be allocated" << Endl;
		break;
	case SCE_NP_AUTH_ESRCH:
		log::error << L"Specified ID does not exist" << Endl;
		break;
	case SCE_NP_AUTH_EBUSY:
		log::error << L"Processing  specified operation cannot be executed" << Endl;
		break;
	case SCE_NP_AUTH_EABORT:
		log::error << L"Operation has been cancelled by the user" << Endl;
		break;
	case SCE_NP_AUTH_EEXIST:
		log::error << L"Specified key already exists" << Endl;
		break;
	case SCE_NP_AUTH_EINVALID_ARGUMENT:
		log::error << L"Invalid argument was specified" << Endl;
		break;
	case CELL_SYSUTIL_ERROR_TYPE:
		log::error << L"Type is invalid" << Endl;
		break;
	case CELL_SYSUTIL_ERROR_VALUE:
		log::error << L"Value is invalid" << Endl;
		break;
	case CELL_SYSUTIL_ERROR_SIZE:
		log::error << L"Size is invalid" << Endl;
		break;
	case CELL_SYSUTIL_ERROR_NUM:
		log::error << L"Number exceeds valid range" << Endl;
		break;
	case CELL_SYSUTIL_ERROR_BUSY:
		log::error << L"Service cannot run because another service is running" << Endl;
		break;
	case CELL_SYSUTIL_ERROR_STATUS:
		log::error << L"Invalid API was called for the current service status" << Endl;
		break;
	case CELL_SYSUTIL_ERROR_MEMORY:
		log::error << L"Sufficient memory could not be allocated for the requested service" << Endl;
		break;
	default:
		if (err >= 0x8002a000 && err <= 0x8002a0ff)
			log::error << L"Internal error" << Endl;
		else
			log::error << L"Undefined trophy error" << Endl;
	}
}

void LogError::logBindErrorSaveData(uint32_t err)
{
	if (err & CELL_SAVEDATA_BINDSTAT_ERR_CONSOLE)
		log::error << L"[Pseudo-bind information] Saved by another PlayStation3" << Endl;

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_DISC)
		log::error << L"[Pseudo-bind information] Saved by an application started on another disc" << Endl;

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_PROGRAM)
		log::error << L"[Pseudo-bind information] Saved by another application (currently unsupported)" << Endl;

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_NOACCOUNTID)
		log::error << L"[Pseudo-bind information] Saved by a user without an account ID" << Endl;

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_ACCOUNTID)
		log::error << L"[Pseudo-bind information] Saved by a user with a different account ID" << Endl;

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_NOUSERID)
		log::error << L"[Pseudo-bind information] User ID is not recorded (Saved with the save data utility of a version before Runtime Library Release 2.0.0)" << Endl;

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_USERID)
		log::error << L"[Pseudo-bind information] Saved by a different user" << Endl;

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_NOOWNER)
		log::error << L"[Owner information] No Owner Information (Created with the save data utility of a version before Runtime Library Release 2.4.0, and owner information was not reset when the data was overwritten)" << Endl;

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_OWNER)
		log::error << L"[Owner information]	Owned by a different user" << Endl;

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_LOCALOWNER)
		log::error << L"[Owner information] No Account ID in Owner Information (Saved when the user was not an online user)" << Endl;
}

}
}
