/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
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

void TaskEnumSaveData::execute(TaskQueue* taskQueue)
{
	T_ASSERT(m_provider);
	T_DEBUG(L"Online; Begin enumerating save data");
	std::set< std::wstring > saveDataIds;
	m_provider->enumerate(saveDataIds);
	(m_sinkObject->*m_sinkMethod)(saveDataIds);
	T_DEBUG(L"Online; Finished enumerating save data");
}

	}
}
