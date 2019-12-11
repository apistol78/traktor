#pragma once

#include "Spark/Action/Common/XMLNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Thread;

	namespace spark
	{

class ActionContext;
struct CallArgs;

/*! ActionScript XML document.
 * \ingroup Spark
 */
class T_DLLCLASS XML : public XMLNode
{
	T_RTTI_CLASS;

public:
	XML(ActionContext* context);

	virtual ~XML();

	bool load(const std::wstring& url);

private:
	ActionContext* m_context;
	Thread* m_thread;

	void onFrame(CallArgs& ca);

	void threadLoad(std::wstring url_);
};

	}
}

