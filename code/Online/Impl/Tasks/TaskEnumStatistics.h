#ifndef traktor_online_TaskEnumStatistics_H
#define traktor_online_TaskEnumStatistics_H

#include <map>
#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IStatisticsProvider;

class TaskEnumStatistics : public ITask
{
	T_RTTI_CLASS;

public:
	typedef void (Object::*sink_method_t)(const std::map< std::wstring, float >&);

	TaskEnumStatistics(
		IStatisticsProvider* provider,
		Object* sinkObject,
		sink_method_t sinkMethod
	);

	virtual void execute();

private:
	Ref< IStatisticsProvider > m_provider;
	Ref< Object > m_sinkObject;
	sink_method_t m_sinkMethod;
};

	}
}

#endif	// traktor_online_TaskEnumStatistics_H
