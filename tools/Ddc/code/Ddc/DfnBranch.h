#ifndef ddc_DfnBranch_H
#define ddc_DfnBranch_H

#include <Core/Heap/Ref.h>
#include "Ddc/DfnNode.h"

namespace ddc
{

class DfnBranch : public DfnNode
{
	T_RTTI_CLASS(DfnBranch)

public:
	DfnBranch(DfnNode* left, DfnNode* right);

	DfnNode* getLeft() const { return m_left; }

	DfnNode* getRight() const { return m_right; }

private:
	traktor::Ref< DfnNode > m_left;
	traktor::Ref< DfnNode > m_right;
};

}

#endif	// ddc_DfnBranch_H
