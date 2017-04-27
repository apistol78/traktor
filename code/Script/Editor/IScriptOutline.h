/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_IScriptOutline_H
#define traktor_script_IScriptOutline_H

#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{
	namespace script
	{

/*! \brief Script outline parser interface.
 * \ingroup Script
 */
class IScriptOutline : public Object
{
	T_RTTI_CLASS;

public:
	class Node : public Object
	{
		T_RTTI_CLASS;

	public:
		Node(int32_t line);

		void setNext(Node* node);

		void setNextTail(Node* node);

		Node* getNext() const;

		int32_t getLine() const;

	private:
		Ref< Node > m_next;
		int32_t m_line;
	};

	class FunctionReferenceNode : public Node
	{
		T_RTTI_CLASS;

	public:
		FunctionReferenceNode(int32_t line, const std::wstring& name);

		const std::wstring& getName() const;

	private:
		std::wstring m_name;
	};

	class FunctionNode : public Node
	{
		T_RTTI_CLASS;

	public:
		FunctionNode(int32_t line, const std::wstring& name, bool local, Node* body);

		const std::wstring& getName() const;

		bool isLocal() const;

		Node* getBody() const;

	private:
		std::wstring m_name;
		bool m_local;
		Ref< Node > m_body;
	};

	virtual Ref< Node > parse(const std::wstring& text) const = 0;
};

	}
}

#endif	// traktor_script_IScriptOutline_H
