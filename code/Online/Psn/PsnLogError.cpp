/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <np.h>
#include <sysutil/sysutil_savedata.h>
#include <sysutil/sysutil_gamecontent.h>
#include "Core/Log/Log.h"
#include "Online/Psn/PsnLogError.h"

namespace traktor
{
	namespace online
	{
namespace
{
}

std::wstring PsnLogError::getSaveDataErrorString(int32_t err)
{
	switch (err)
	{
	case CELL_SAVEDATA_ERROR_CBRESULT:
		return L"Callback function returned an error";
	case CELL_SAVEDATA_ERROR_ACCESS_ERROR:
		return L"HDD access error";
	case CELL_SAVEDATA_ERROR_INTERNAL:
		return L"Fatal internal error";
	case CELL_SAVEDATA_ERROR_PARAM:
		return L"Error in parameter to be set to utility (application bug)";
	case CELL_SAVEDATA_ERROR_NOSPACE:
		return L"Insufficient free space (application bug: lack of free space must be judged and handled within the callback function)";
	case CELL_SAVEDATA_ERROR_BROKEN:
		return L"Save data corrupted (modification detected, etc.)";
	case CELL_SAVEDATA_ERROR_FAILURE:
		return L"Save/load of save data failed (file could not be found, etc.)";
	case CELL_SAVEDATA_ERROR_BUSY:
		return L"Save data utility function was called simultaneously";
	default:
		return L"Undefined error during save";
	}
}

std::wstring PsnLogError::getTrophyErrorString(int32_t err)
{
	switch (err)
	{
	case SCE_NP_TROPHY_ERROR_NOT_INITIALIZED:
		return L"Not initialized. sceNpTrophyInit() has not been called, or sceNpTrophyTerm()has already been called. Check the calling order.";
	case SCE_NP_TROPHY_ERROR_INVALID_ARGUMENT:
		return L"Invalid argument. NULL was specified to statusCb. Check the argument value.";
	case SCE_NP_TROPHY_ERROR_UNKNOWN_CONTEXT:
		return L"Unknown context. The specified context does not exist or it has already been destroyed. Check context.";
	case SCE_NP_TROPHY_ERROR_PROCESSING_ABORTED:
		return L"An invalid value was returned from SceNpTrophyStatusCallback and further processing has been aborted.";
	case SCE_NP_TROPHY_ERROR_ABORT:
		return L"The handle has been aborted. The processing has been aborted by sceNpTrophyAbortHandle(). Destroy handle. ";
	case SCE_NP_TROPHY_ERROR_UNKNOWN_HANDLE:
		return L"Unknown handle. The specified handle does not exist or it has already been destroyed. Check handle.";
	case SCE_NP_TROPHY_ERROR_INVALID_NP_COMM_ID:
		return L"Invalid NP Communication ID. Either the NP Communication ID or the NP Communication Signature is invalid. Check the argument values for when the context was created.";
	case SCE_NP_TROPHY_ERROR_UNKNOWN_NP_COMM_ID:
		return L"NP Communication IDs do not match. The specified NP Communication ID does not match the NP Communication ID in the trophy pack file. Check the argument values for when the context was created.";
	case SCE_NP_TROPHY_ERROR_CONF_DOES_NOT_EXIST:
		return L"Trophy pack file does not exist";
	case SCE_NP_TROPHY_ERROR_VERIFICATION_FAILURE:
		return L"Trophy pack file signature verification failed. The trophy pack file could not be properly verified. Be sure to access the Trophy Authoring Tool and append a digital signature to the trophy pack file.";
	case SCE_NP_TROPHY_ERROR_SHUTDOWN:
		return L"The system is being shut down. The processing cannot be carried on and is therefore being canceled.";
	case SCE_NP_TROPHY_ERROR_TITLE_ICON_NOT_FOUND:
		return L"Trophy set still image icon does not exist. Check that the trophy pack file contains the trophy set still image icon.";
	case SCE_NP_TROPHY_ERROR_TROPHY_ICON_NOT_FOUND:
		return L"Trophy still image icon icon does not exist.Check that the trophy pack file contains all the trophy still image icons.";
	case SCE_NP_TROPHY_ERROR_INSUFFICIENT_DISK_SPACE:
		return L"Insufficient disk space. There is not enough disk space to register the trophy set to the internal HDD. Notify the user accordingly. ";
	case SCE_NP_TROPHY_ERROR_ILLEGAL_UPDATE:
		return L"Cannot update. An irresolvable discrepancy has been found between the registered data and the data trying to be registered.  Possible causes are from a change in trophy grade or a reduction in the number of trophies from the previous install. Check the trophy configuration data, and remove the previously installed trophy set from the console.";
	case SCE_NP_AUTH_EINVAL:
		return L"Timing of the function call is invalid";
	case SCE_NP_AUTH_ENOMEM:
		return L"Insufficient memory - memory could not be allocated";
	case SCE_NP_AUTH_ESRCH:
		return L"Specified ID does not exist";
	case SCE_NP_AUTH_EBUSY:
		return L"Processing  specified operation cannot be executed";
	case SCE_NP_AUTH_EABORT:
		return L"Operation has been cancelled by the user";
	case SCE_NP_AUTH_EEXIST:
		return L"Specified key already exists";
	case SCE_NP_AUTH_EINVALID_ARGUMENT:
		return L"Invalid argument was specified";
	case CELL_SYSUTIL_ERROR_TYPE:
		return L"Type is invalid";
	case CELL_SYSUTIL_ERROR_VALUE:
		return L"Value is invalid";
	case CELL_SYSUTIL_ERROR_SIZE:
		return L"Size is invalid";
	case CELL_SYSUTIL_ERROR_NUM:
		return L"Number exceeds valid range";
	case CELL_SYSUTIL_ERROR_BUSY:
		return L"Service cannot run because another service is running";
	case CELL_SYSUTIL_ERROR_STATUS:
		return L"Invalid API was called for the current service status";
	case CELL_SYSUTIL_ERROR_MEMORY:
		return L"Sufficient memory could not be allocated for the requested service";
	default:
		if (err >= 0x8002a000 && err <= 0x8002a0ff)
			return L"Internal error";
		else
			return L"Undefined trophy error";
	}
}

std::wstring PsnLogError::getSaveDataBindErrorString(uint32_t err)
{
	std::wstring ret;
	if (err & CELL_SAVEDATA_BINDSTAT_ERR_CONSOLE)
		ret += L"[Pseudo-bind information] Saved by another PlayStation3\n";

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_DISC)
		ret += L"[Pseudo-bind information] Saved by an application started on another disc\n";

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_PROGRAM)
		ret += L"[Pseudo-bind information] Saved by another application (currently unsupported)\n";

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_NOACCOUNTID)
		ret += L"[Pseudo-bind information] Saved by a user without an account ID\n";

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_ACCOUNTID)
		ret += L"[Pseudo-bind information] Saved by a user with a different account ID\n";

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_NOUSERID)
		ret += L"[Pseudo-bind information] User ID is not recorded (Saved with the save data utility of a version before Runtime Library Release 2.0.0)\n";

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_USERID)
		ret += L"[Pseudo-bind information] Saved by a different user\n";

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_NOOWNER)
		ret += L"[Owner information] No Owner Information (Created with the save data utility of a version before Runtime Library Release 2.4.0, and owner information was not reset when the data was overwritten)\n";

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_OWNER)
		ret += L"[Owner information]	Owned by a different user\n";

	if (err & CELL_SAVEDATA_BINDSTAT_ERR_LOCALOWNER)
		ret += L"[Owner information] No Account ID in Owner Information (Saved when the user was not an online user)\n";
	return ret;
}

	}
}
