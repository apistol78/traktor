/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"
#include "Core/Thread/Ps3/Spurs/SpursManager.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Singleton/SingletonManager.h"
#include "Spray/Ps3/SprayJobQueue.h"

namespace traktor
{
	namespace spray
	{

SprayJobQueue& SprayJobQueue::getInstance()
{
	static SprayJobQueue* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new SprayJobQueue();
		SingletonManager::getInstance().addBefore(s_instance, &SpursManager::getInstance());
	}
	return *s_instance;
}

SpursJobQueue* SprayJobQueue::getJobQueue() const
{
	return m_jobQueue;
}

void SprayJobQueue::destroy()
{
	delete this;
}

SprayJobQueue::SprayJobQueue()
{
	m_jobQueue = SpursManager::getInstance().createJobQueue(128, 256, SpursManager::Normal);
}

SprayJobQueue::~SprayJobQueue()
{
	safeDestroy(m_jobQueue);
}

	}
}
