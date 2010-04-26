#ifndef traktor_flash_AsXMLNode_H
#define traktor_flash_AsXMLNode_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief XML node class.
 * \ingroup Flash
 */
class AsXMLNode : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< AsXMLNode > getInstance();

private:
	AsXMLNode();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const ActionValueArray& args);

	void XMLNode_appendChild(CallArgs& ca);

	void XMLNode_cloneNode(CallArgs& ca);

	void XMLNode_getNamespaceForPrefix(CallArgs& ca);

	void XMLNode_getPrefixForNamespace(CallArgs& ca);

	void XMLNode_hasChildNodes(CallArgs& ca);

	void XMLNode_insertBefore(CallArgs& ca);

	void XMLNode_removeNode(CallArgs& ca);

	void XMLNode_toString(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsXMLNode_H
