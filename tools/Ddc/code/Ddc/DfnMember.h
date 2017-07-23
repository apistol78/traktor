/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ddc_DfnMember_H
#define ddc_DfnMember_H

#include <Core/Ref.h>
#include "Ddc/DfnNode.h"

namespace ddc
{

class DfnMember : public DfnNode
{
	T_RTTI_CLASS;

public:
	DfnMember();

	DfnMember(DfnNode* type, const std::wstring& name);

	virtual bool serialize(traktor::ISerializer& s);

	const DfnNode* getMemberType() const { return m_type; }

	const std::wstring& getName() const { return m_name; }

private:
	traktor::Ref< DfnNode > m_type;
	std::wstring m_name;
};

}

#endif	// ddc_DfnMember_H
