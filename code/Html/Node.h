#ifndef traktor_html_Node_H
#define traktor_html_Node_H

#include <string>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HTML_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Stream;

	namespace html
	{

/*! \brief HTML DOM Node.
 * \ingroup HTML
 */
class T_DLLCLASS Node : public Object
{
	T_RTTI_CLASS(Node)

public:
	virtual std::wstring getName() const;

	virtual std::wstring getValue() const;

	void addChild(Node* child);
	
	void insertBefore(Node* child, Node* node);
	
	void insertAfter(Node* child, Node* node);

	Node* getParent() const;
	
	Node* getPreviousSibling() const;
	
	Node* getNextSibling() const;
	
	Node* getFirstChild() const;
	
	Node* getLastChild() const;

	virtual void writeHtml(Stream* stream);

private:
	Ref< Node > m_parent;
	Ref< Node > m_previousSibling;
	Ref< Node > m_nextSibling;
	Ref< Node > m_firstChild;
	Ref< Node > m_lastChild;
};

	}
}

#endif	// traktor_html_Node_H
