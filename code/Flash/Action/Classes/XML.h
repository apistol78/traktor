#ifndef traktor_flash_XML_H
#define traktor_flash_XML_H

#include "Flash/Action/Classes/XMLNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionContext;

/*! \brief ActionScript XML document.
 * \ingroup Flash
 */
class T_DLLCLASS XML : public XMLNode
{
	T_RTTI_CLASS;

public:
	XML(ActionContext* context);

	bool load(const std::wstring& url);

private:
	ActionContext* m_context;
};

	}
}

#endif	// traktor_flash_XML_H
