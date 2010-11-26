#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/MemberRef.h>
#include "Ddc/DfnNode.h"

using namespace traktor;

namespace ddc
{

T_IMPLEMENT_RTTI_CLASS(L"ddc.DfnNode", DfnNode, ISerializable)

bool DfnNode::serialize(ISerializer& s)
{
	s >> MemberRef< DfnNode >(L"next", m_next);
	return true;
}

}
