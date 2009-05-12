#ifndef traktor_xml_Node_H
#define traktor_xml_Node_H

#include <string>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_XML_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace xml
	{

/*! \brief XML Node.
 * \ingroup XML
 */
class T_DLLCLASS Node : public Object
{
	T_RTTI_CLASS(Node)

public:
	virtual std::wstring getName() const;

	virtual void setName(const std::wstring& name);

	virtual std::wstring getValue() const;

	virtual void setValue(const std::wstring& value);

	void addChild(Node* child);
	
	void insertBefore(Node* child, Node* node);
	
	void insertAfter(Node* child, Node* node);

	Node* getParent() const;
	
	Node* getPreviousSibling() const;
	
	Node* getNextSibling() const;
	
	Node* getFirstChild() const;
	
	Node* getLastChild() const;

private:
	Ref< Node > m_parent;
	Ref< Node > m_previousSibling;
	Ref< Node > m_nextSibling;
	Ref< Node > m_firstChild;
	Ref< Node > m_lastChild;
};

	}
}

#endif	// traktor_xml_Node_H
