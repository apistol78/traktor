#ifndef ddc_DfnMember_H
#define ddc_DfnMember_H

#include <Core/Heap/Ref.h>
#include "Ddc/DfnNode.h"

namespace ddc
{

class DfnMember : public DfnNode
{
	T_RTTI_CLASS(DfnMember)

public:
	DfnMember(DfnNode* type, const std::wstring& name);

	const DfnNode* getMemberType() const { return m_type; }

	const std::wstring& getName() const { return m_name; }

private:
	traktor::Ref< DfnNode > m_type;
	std::wstring m_name;
};

}

#endif	// ddc_DfnMember_H
