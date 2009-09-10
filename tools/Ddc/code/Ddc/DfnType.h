#ifndef ddc_DfnType_H
#define ddc_DfnType_H

#include "Ddc/DfnNode.h"

namespace ddc
{

class DfnType : public DfnNode
{
	T_RTTI_CLASS(DfnType)

public:
	DfnType(const std::wstring& name, bool isArray);

	const std::wstring& getName() const { return m_name; }

	bool isArray() const { return m_isArray; }

private:
	std::wstring m_name;
	bool m_isArray;
};

}

#endif	// ddc_DfnType_H
