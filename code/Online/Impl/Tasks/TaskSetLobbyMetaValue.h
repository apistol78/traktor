/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_TaskSetLobbyMetaValue_H
#define traktor_online_TaskSetLobbyMetaValue_H

#include <string>
#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class Result;

class TaskSetLobbyMetaValue : public ITask
{
	T_RTTI_CLASS;

public:
	TaskSetLobbyMetaValue(
		IMatchMakingProvider* provider,
		uint64_t lobbyHandle,
		const std::wstring& key,
		const std::wstring& value,
		Result* result
	);

	virtual void execute(TaskQueue* taskQueue) T_OVERRIDE T_FINAL;

private:
	Ref< IMatchMakingProvider > m_provider;
	uint64_t m_lobbyHandle;
	std::wstring m_key;
	std::wstring m_value;
	Ref< Result > m_result;
};

	}
}

#endif	// traktor_online_TaskSetLobbyMetaValue_H
