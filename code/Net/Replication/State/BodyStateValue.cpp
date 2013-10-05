#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Net/Replication/State/BodyStateValue.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

class MemberBodyState : public MemberComplex
{
public:
	MemberBodyState(const wchar_t* const name, physics::BodyState& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		if (s.getDirection() == ISerializer::SdRead)
		{
			Transform transform;
			Vector4 linearVelocity;
			Vector4 angularVelocity;

			s >> MemberComposite< Transform >(L"transform", transform);
			s >> Member< Vector4 >(L"linearVelocity", linearVelocity, AttributeDirection());
			s >> Member< Vector4 >(L"angularVelocity", angularVelocity, AttributeDirection());

			m_ref.setTransform(transform);
			m_ref.setLinearVelocity(linearVelocity);
			m_ref.setAngularVelocity(angularVelocity);
		}
		else	// SdWrite
		{
			Transform transform = m_ref.getTransform();
			Vector4 linearVelocity = m_ref.getLinearVelocity();
			Vector4 angularVelocity = m_ref.getAngularVelocity();

			s >> MemberComposite< Transform >(L"transform", transform);
			s >> Member< Vector4 >(L"linearVelocity", linearVelocity, AttributeDirection());
			s >> Member< Vector4 >(L"angularVelocity", angularVelocity, AttributeDirection());
		}
	}

private:
	physics::BodyState& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.net.BodyStateValue", 0, BodyStateValue, IValue)

void BodyStateValue::serialize(ISerializer& s)
{
	s >> MemberBodyState(L"value", m_value);
}

	}
}
