#ifndef traktor_xml_Node_H
#define traktor_xml_Node_H

#include <string>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_XML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

class OutputStream;

	namespace xml
	{

/*! \brief XML Node.
 * \ingroup XML
 */
class T_DLLCLASS Node : public Object
{
	T_RTTI_CLASS;

public:
	Node();

	virtual ~Node();

	virtual std::wstring getName() const;

	virtual void setName(const std::wstring& name);

	virtual std::wstring getValue() const;

	virtual void setValue(const std::wstring& value);

	virtual void write(OutputStream& os) const;

	void unlink();

	void addChild(Node* child);

	void removeChild(Node* child);

	void removeAllChildren();
	
	void insertBefore(Node* child, Node* beforeNode);
	
	void insertAfter(Node* child, Node* afterNode);

	Node* getParent() const;
	
	Node* getPreviousSibling() const;
	
	Node* getNextSibling() const;
	
	Node* getFirstChild() const;
	
	Node* getLastChild() const;

protected:
	virtual Ref< Node > cloneUntyped() const = 0;

	void cloneChildren(Node* clone) const;

private:
	Node* m_parent;
	Node* m_previousSibling;
	Ref< Node > m_nextSibling;
	Ref< Node > m_firstChild;
	Ref< Node > m_lastChild;
};

	}
}

#endif	// traktor_xml_Node_H
