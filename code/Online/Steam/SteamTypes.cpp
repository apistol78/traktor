/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/String.h"
#include "Online/Steam/SteamTypes.h"

namespace traktor
{
	namespace online
	{

std::wstring getSteamError(EResult result)
{
	switch (result)
	{
	case k_EResultOK:
		return L"k_EResultOK";
	case k_EResultFail:
		return L"k_EResultFail";
	case k_EResultNoConnection:
		return L"k_EResultNoConnection";
	case k_EResultInvalidPassword:
		return L"k_EResultInvalidPassword";
	case k_EResultLoggedInElsewhere:
		return L"k_EResultLoggedInElsewhere";
	case k_EResultInvalidProtocolVer:
		return L"k_EResultInvalidProtocolVer";
	case k_EResultInvalidParam:
		return L"k_EResultInvalidParam";
	case k_EResultFileNotFound:
		return L"k_EResultFileNotFound";
	case k_EResultBusy:
		return L"k_EResultBusy";
	case k_EResultInvalidState:
		return L"k_EResultInvalidState";
	case k_EResultInvalidName:
		return L"k_EResultInvalidName";
	case k_EResultInvalidEmail:
		return L"k_EResultInvalidEmail";
	case k_EResultDuplicateName:
		return L"k_EResultDuplicateName";
	case k_EResultAccessDenied:
		return L"k_EResultAccessDenied";
	case k_EResultTimeout:
		return L"k_EResultTimeout";
	case k_EResultBanned:
		return L"k_EResultBanned";
	case k_EResultAccountNotFound:
		return L"k_EResultAccountNotFound";
	case k_EResultInvalidSteamID:
		return L"k_EResultInvalidSteamID";
	case k_EResultServiceUnavailable:
		return L"k_EResultServiceUnavailable";
	case k_EResultNotLoggedOn:
		return L"k_EResultNotLoggedOn";
	case k_EResultPending:
		return L"k_EResultPending";
	case k_EResultEncryptionFailure:
		return L"k_EResultEncryptionFailure";
	case k_EResultInsufficientPrivilege:
		return L"k_EResultInsufficientPrivilege";
	case k_EResultLimitExceeded:
		return L"k_EResultLimitExceeded";
	case k_EResultRevoked:
		return L"k_EResultRevoked";
	case k_EResultExpired:
		return L"k_EResultExpired";
	case k_EResultAlreadyRedeemed:
		return L"k_EResultAlreadyRedeemed";
	case k_EResultDuplicateRequest:
		return L"k_EResultDuplicateRequest";
	case k_EResultAlreadyOwned:
		return L"k_EResultAlreadyOwned";
	case k_EResultIPNotFound:
		return L"k_EResultIPNotFound";
	case k_EResultPersistFailed:
		return L"k_EResultPersistFailed";
	case k_EResultLockingFailed:
		return L"k_EResultLockingFailed";
	case k_EResultLogonSessionReplaced:
		return L"k_EResultLogonSessionReplaced";
	case k_EResultConnectFailed:
		return L"k_EResultConnectFailed";
	case k_EResultHandshakeFailed:
		return L"k_EResultHandshakeFailed";
	case k_EResultIOFailure:
		return L"k_EResultIOFailure";
	case k_EResultRemoteDisconnect:
		return L"k_EResultRemoteDisconnect";
	case k_EResultShoppingCartNotFound:
		return L"k_EResultShoppingCartNotFound";
	case k_EResultBlocked:
		return L"k_EResultBlocked";
	case k_EResultIgnored:
		return L"k_EResultIgnored";
	case k_EResultNoMatch:
		return L"k_EResultNoMatch";
	case k_EResultAccountDisabled:
		return L"k_EResultAccountDisabled";
	case k_EResultServiceReadOnly:
		return L"k_EResultServiceReadOnly";
	case k_EResultAccountNotFeatured:
		return L"k_EResultAccountNotFeatured";
	case k_EResultAdministratorOK:
		return L"k_EResultAdministratorOK";
	case k_EResultContentVersion:
		return L"k_EResultContentVersion";
	case k_EResultTryAnotherCM:
		return L"k_EResultTryAnotherCM";
	case k_EResultPasswordRequiredToKickSession:
		return L"k_EResultPasswordRequiredToKickSession";
	case k_EResultAlreadyLoggedInElsewhere:
		return L"k_EResultAlreadyLoggedInElsewhere";
	case k_EResultSuspended:
		return L"k_EResultSuspended";
	case k_EResultCancelled:
		return L"k_EResultCancelled";
	case k_EResultDataCorruption:
		return L"k_EResultDataCorruption";
	case k_EResultDiskFull:
		return L"k_EResultDiskFull";
	case k_EResultRemoteCallFailed:
		return L"k_EResultRemoteCallFailed";
	}
	return toString(int(result));
}

	}
}
