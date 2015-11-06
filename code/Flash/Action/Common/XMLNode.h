#ifndef traktor_flash_XMLNode_H
#define traktor_flash_XMLNode_H

#include "Flash/Action/ActionObjectRelay.h"

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

/*! \brief ActionScript XML node.
 * \ingroup Flash
 */
class T_DLLCLASS XMLNode : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	enum NodeType
	{
		NtUnknown = 0,
		NtElement = 1,
		NtAttribute = 2,
		NtText = 3,
		NtCDataSection = 4,
		NtEntityReference = 5,
		NtEntity = 6,
		NtProcessingInstruction = 7,
		NtComment = 8,
		NtDocument = 9,
		NtDocumentType = 10,
		NtDocumentFragment = 11,
		NtNotation = 12
	};

	XMLNode(
		const char* const prototype,
		NodeType nodeType,
		const std::wstring& localName,
		const std::wstring& nodeValue
	);

	NodeType getNodeType() const;

	const std::wstring& getLocalName() const;

	const std::wstring& getNodeName() const;

	const std::wstring& getNodeValue() const;

	const std::wstring& getNamespaceURI() const;

	const std::wstring& getPrefix() const;

	void appendChild(XMLNode* node);

	void insertBefore(XMLNode* node, const XMLNode* before);

	void removeNode();

	Ref< XMLNode > cloneNode() const;

	XMLNode* getFirstChild() const;

	XMLNode* getLastChild() const;

	XMLNode* getParentNode() const;

	XMLNode* getNextSibling() const;

	XMLNode* getPreviousSibling() const;

protected:
	virtual void trace(visitor_t visitor) const T_OVERRIDE T_FINAL;

	virtual void dereference() T_OVERRIDE T_FINAL;

private:
	NodeType m_nodeType;
	std::wstring m_localName;
	std::wstring m_nodeName;
	std::wstring m_nodeValue;
	std::wstring m_namespaceURI;
	std::wstring m_prefix;
	
	XMLNode* m_parent;
	Ref< XMLNode > m_firstChild;
	Ref< XMLNode > m_lastChild;
	XMLNode* m_previousSibling;
	Ref< XMLNode > m_nextSibling;
};

	}
}

#endif	// traktor_flash_XMLNode_H
