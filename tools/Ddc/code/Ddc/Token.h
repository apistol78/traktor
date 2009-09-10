#ifndef ddc_Token_H
#define ddc_Token_H

#include <Core/Heap/Ref.h>

namespace ddc
{

class DfnNode;

struct Token
{
	std::wstring literal;
	traktor::Ref< DfnNode > node;

	Token()
	{
	}

	explicit Token(const std::wstring& _literal)
	:	literal(_literal)
	{
	}

	explicit Token(DfnNode* _node)
	:	node(_node)
	{
	}
};

}

#endif	// ddc_Token_H
