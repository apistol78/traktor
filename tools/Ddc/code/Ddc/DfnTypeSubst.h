/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ddc_DfnTypeSubst_H
#define ddc_DfnTypeSubst_H

#include <Core/Ref.h>
#include "Ddc/DfnNode.h"

namespace ddc
{

class DfnTypeSubst : public DfnNode
{
	T_RTTI_CLASS;

public:
	DfnTypeSubst();

	DfnTypeSubst(const std::wstring& tag);

	virtual bool serialize(traktor::ISerializer& s);

	const std::wstring& getTag() const { return m_tag; }

private:
	std::wstring m_tag;
};

}

#endif	// ddc_DfnTypeSubst_H
