/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <sysutil/sysutil_common.h>
#include "Core/Singleton/SingletonManager.h"
#include "Core/System/Ps3/SystemCallback.h"

namespace traktor
{

SystemCallback& SystemCallback::getInstance()
{
	static SystemCallback* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new SystemCallback();
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

void SystemCallback::add(callback_t callback)
{
	m_callbacks.push_back(callback);
}

void SystemCallback::update()
{
	cellSysutilCheckCallback();
}

SystemCallback::SystemCallback()
{
	cellSysutilRegisterCallback(0, systemCallback, this);
}

SystemCallback::~SystemCallback()
{
	cellSysutilUnregisterCallback(0);
}

void SystemCallback::destroy()
{
	delete this;
}

void SystemCallback::systemCallback(uint64_t status, uint64_t param, void* userData)
{
	SystemCallback* this_ = static_cast< SystemCallback* >(userData);
	T_ASSERT (this_);

	for (std::vector< callback_t >::iterator i = this_->m_callbacks.begin(); i != this_->m_callbacks.end(); ++i)
		(*i)(status, param);
}

}
