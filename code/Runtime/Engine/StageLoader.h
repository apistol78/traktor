#pragma once

#include "Core/Object.h"
#include "Core/Guid.h"

namespace traktor
{

class Job;

	namespace runtime
	{

class IEnvironment;

	}

	namespace runtime
	{

class Stage;

/*! \brief
 * \ingroup Runtime
 */
class StageLoader : public Object
{
	T_RTTI_CLASS;

public:
	bool wait();

	bool ready();

	bool succeeded();

	bool failed();

	Ref< Stage > get();

private:
	friend class Stage;

	Ref< Stage > m_stage;
	Ref< Job > m_job;

	static Ref< StageLoader > createAsync(IEnvironment* environment, const Guid& stageGuid, const Object* params);

	static Ref< StageLoader > create(IEnvironment* environment, const Guid& stageGuid, const Object* params);
};

	}
}

