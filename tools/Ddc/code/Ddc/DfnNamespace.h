#ifndef ddc_DfnNamespace_H
#define ddc_DfnNamespace_H

#include <Core/Heap/Ref.h>
#include "Ddc/DfnNode.h"

namespace ddc
{

class DfnNamespace : public DfnNode
{
	T_RTTI_CLASS(DfnNamespace)

public:
	DfnNamespace(const std::wstring& name, DfnNode* statements);

	const std::wstring& getName() const { return m_name; }

	const DfnNode* getStatements() const { return m_statements; }

private:
	std::wstring m_name;
	traktor::Ref< DfnNode > m_statements;
};

}

#endif	// ddc_DfnNamespace_H
