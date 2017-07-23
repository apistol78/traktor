/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ddc_DfnClass_H
#define ddc_DfnClass_H

#include <Core/Ref.h>
#include "Ddc/DfnNode.h"

namespace ddc
{

class DfnClass : public DfnNode
{
	T_RTTI_CLASS;

public:
	enum Access
	{
		AccPrivate,
		AccPublic
	};

	DfnClass();

	DfnClass(Access access, const std::wstring& name, DfnNode* members);

	DfnClass(Access access, const std::wstring& name, const std::wstring& super, DfnNode* members);

	virtual bool serialize(traktor::ISerializer& s);

	Access getAccess() const { return m_access; }

	const std::wstring& getName() const { return m_name; }

	const std::wstring& getSuper() const { return m_super; }

	const DfnNode* getMembers() const { return m_members; }

private:
	Access m_access;
	std::wstring m_name;
	std::wstring m_super;
	traktor::Ref< DfnNode > m_members;
};

}

#endif	// ddc_DfnClass_H
