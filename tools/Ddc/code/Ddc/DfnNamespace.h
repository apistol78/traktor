/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ddc_DfnNamespace_H
#define ddc_DfnNamespace_H

#include <Core/Ref.h>
#include "Ddc/DfnNode.h"

namespace ddc
{

class DfnNamespace : public DfnNode
{
	T_RTTI_CLASS;

public:
	DfnNamespace();

	DfnNamespace(const std::wstring& name, DfnNode* statements);

	virtual bool serialize(traktor::ISerializer& s);

	const std::wstring& getName() const { return m_name; }

	const DfnNode* getStatements() const { return m_statements; }

private:
	std::wstring m_name;
	traktor::Ref< DfnNode > m_statements;
};

}

#endif	// ddc_DfnNamespace_H
