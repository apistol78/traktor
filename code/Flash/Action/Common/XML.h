#ifndef traktor_flash_XML_H
#define traktor_flash_XML_H

#include "Flash/Action/Common/XMLNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Thread;

	namespace flash
	{

class ActionContext;
struct CallArgs;

/*! \brief ActionScript XML document.
 * \ingroup Flash
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

#endif	// traktor_flash_XML_H
