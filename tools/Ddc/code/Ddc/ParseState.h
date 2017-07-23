/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ddc_ParseState_H
#define ddc_ParseState_H

#include <Core/Ref.h>

namespace ddc
{

class DfnNode;

struct ParseState
{
	traktor::Ref< DfnNode > root;
};

}

#endif	// ddc_ParseState_H
