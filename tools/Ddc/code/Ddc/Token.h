/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ddc_Token_H
#define ddc_Token_H

#include <Core/Ref.h>

namespace ddc
{

class DfnNode;

struct Token
{
	std::wstring literal;
	double number;
	traktor::Ref< DfnNode > node;

	Token()
	:	number(0.0)
	{
	}

	explicit Token(const std::wstring& _literal)
	:	literal(_literal)
	,	number(0.0)
	{
	}

	explicit Token(double _number)
	:	number(_number)
	{
	}

	explicit Token(DfnNode* _node)
	:	number(0.0)
	,	node(_node)
	{
	}
};

}

#endif	// ddc_Token_H
