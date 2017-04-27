/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_SprayJobQueue_H
#define traktor_spray_SprayJobQueue_H

#include "Core/Singleton/ISingleton.h"

namespace traktor
{

class SpursJobQueue;

	namespace spray
	{

class SprayJobQueue : public ISingleton
{
public:
	static SprayJobQueue& getInstance();

	SpursJobQueue* getJobQueue() const;

protected:
	virtual void destroy();

private:
	Ref< SpursJobQueue > m_jobQueue;

	SprayJobQueue();

	virtual ~SprayJobQueue();
};

	}
}

#endif	// traktor_spray_SprayJobQueue_H
