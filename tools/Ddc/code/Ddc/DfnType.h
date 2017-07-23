/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ddc_DfnType_H
#define ddc_DfnType_H

#include "Ddc/DfnNode.h"

namespace ddc
{

class DfnType : public DfnNode
{
	T_RTTI_CLASS;

public:
	DfnType();

	DfnType(const std::wstring& name, DfnNode* subst, bool isArray = false, int32_t range = -1);

	virtual bool serialize(traktor::ISerializer& s);

	const std::wstring& getName() const { return m_name; }

	const DfnNode* getSubst() const { return m_subst; }

	bool isArray() const { return m_isArray; }

	bool haveRange() const { return m_range >= 0; }

	int32_t getRange() const { return m_range; }

private:
	std::wstring m_name;
	traktor::Ref< DfnNode > m_subst;
	bool m_isArray;
	int32_t m_range;
};

}

#endif	// ddc_DfnType_H
