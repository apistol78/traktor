#ifndef ddc_DfnClass_H
#define ddc_DfnClass_H

#include <Core/Heap/Ref.h>
#include "Ddc/DfnNode.h"

namespace ddc
{

class DfnClass : public DfnNode
{
	T_RTTI_CLASS(DfnClass)

public:
	enum Access
	{
		AccPrivate,
		AccPublic
	};

	DfnClass(Access access, const std::wstring& name, DfnNode* members);

	DfnClass(Access access, const std::wstring& name, const std::wstring& super, DfnNode* members);

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
