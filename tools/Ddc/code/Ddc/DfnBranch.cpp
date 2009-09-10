#include "Ddc/DfnBranch.h"

using namespace traktor;

namespace ddc
{

T_IMPLEMENT_RTTI_CLASS(L"ddc.DfnBranch", DfnBranch, Object)

DfnBranch::DfnBranch(DfnNode* left, DfnNode* right)
:	m_left(left)
,	m_right(right)
{
}

}
