#include "Net/Replication/State/BodyStateTemplate.h"
#include "Net/Replication/State/BodyStateValue.h"
#include "Net/Replication/State/FloatTemplate.h"
#include "Net/Replication/State/FloatValue.h"
#include "Net/Replication/State/State.h"
#include "Net/Replication/State/StateTemplate.h"
#include "UnitTest/CaseValueTemplate.h"
#include "UnitTest/MathCompare.h"

namespace traktor
{
	namespace
	{

Ref< const net::State > transfer(const net::StateTemplate* st, const net::State* s)
{
	uint8_t buffer[1024];
	uint32_t size = st->pack(s, buffer, sizeof(buffer));
	return st->unpack(buffer, size);
}

	}

void CaseValueTemplate::run()
{
	{
		net::StateTemplate st;
		st.declare(new net::FloatTemplate(1.0f));

		net::State s0;
		s0.pack< net::FloatValue >(0.0f);

		net::State s1;
		s1.pack< net::FloatValue >(1.0f);

		net::State s2;
		s2.pack< net::FloatValue >(2.0f);

		Ref< const net::State > S0 = transfer(&st, &s0);
		Ref< const net::State > S1 = transfer(&st, &s1);
		Ref< const net::State > S2 = transfer(&st, &s2);

		Ref< const net::State > r0 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 0.0f);
		Ref< const net::State > r1 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 0.5f);
		Ref< const net::State > r2 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 1.0f);
		Ref< const net::State > r3 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 1.5f);
		Ref< const net::State > r4 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 2.0f);

		float f0 = r0->getValue< net::FloatValue >(0);
		CASE_ASSERT_COMPARE(f0, 0.0f, fuzzyEqual);

		float f1 = r1->getValue< net::FloatValue >(0);
		CASE_ASSERT_COMPARE(f1, 1.0f, fuzzyEqual);

		float f2 = r2->getValue< net::FloatValue >(0);
		CASE_ASSERT_COMPARE(f2, 2.0f, fuzzyEqual);

		float f3 = r3->getValue< net::FloatValue >(0);
		CASE_ASSERT_COMPARE(f3, 3.0f, fuzzyEqual);

		float f4 = r4->getValue< net::FloatValue >(0);
		CASE_ASSERT_COMPARE(f4, 4.0f, fuzzyEqual);
	}

	{
		net::StateTemplate st;
		st.declare(new net::FloatTemplate(1.0f, 0.0f, 2.5f, true));

		net::State s0;
		s0.pack< net::FloatValue >(0.0f);

		net::State s1;
		s1.pack< net::FloatValue >(1.0f);

		net::State s2;
		s2.pack< net::FloatValue >(2.0f);

		Ref< const net::State > S0 = transfer(&st, &s0);
		Ref< const net::State > S1 = transfer(&st, &s1);
		Ref< const net::State > S2 = transfer(&st, &s2);

		Ref< const net::State > r0 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 0.0f);
		Ref< const net::State > r1 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 0.5f);
		Ref< const net::State > r2 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 1.0f);
		Ref< const net::State > r3 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 1.5f);
		Ref< const net::State > r4 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 2.0f);

		float f0 = r0->getValue< net::FloatValue >(0);
		CASE_ASSERT_COMPARE(f0, 0.0f, fuzzyEqual);

		float f1 = r1->getValue< net::FloatValue >(0);
		CASE_ASSERT_COMPARE(f1, 1.0f, fuzzyEqual);

		float f2 = r2->getValue< net::FloatValue >(0);
		CASE_ASSERT_COMPARE(f2, 2.0f, fuzzyEqual);

		float f3 = r3->getValue< net::FloatValue >(0);
		CASE_ASSERT_COMPARE(f3, 2.5f, fuzzyEqual);

		float f4 = r4->getValue< net::FloatValue >(0);
		CASE_ASSERT_COMPARE(f4, 2.5f, fuzzyEqual);
	}

	{
		physics::BodyState bs0, bs1, bs2;

		bs0.setTransform(Transform(
			Vector4(-1.0f, -2.0f, -3.0f, 1.0f),
			Quaternion::fromEulerAngles(
				deg2rad(90.0f),
				deg2rad(0.0f),
				deg2rad(0.0f)
			)
		));
		bs0.setLinearVelocity(Vector4(-1.0f, -2.0f, -3.0f, 0.0f));
		bs0.setAngularVelocity(Vector4(1.0f, 0.0f, 0.0f, 0.0f));

		bs1.setTransform(Transform(
			Vector4(1.0f, 2.0f, 3.0f, 1.0f),
			Quaternion::fromEulerAngles(
				deg2rad(0.0f),
				deg2rad(90.0f),
				deg2rad(0.0f)
			)
		));
		bs1.setLinearVelocity(Vector4(-4.0f, -5.0f, -6.0f, 0.0f));
		bs1.setAngularVelocity(Vector4(0.0f, 1.0f, 0.0f, 0.0f));

		bs2.setTransform(Transform(
			Vector4(-3.0f, -4.0f, -5.0f, 1.0f),
			Quaternion::fromEulerAngles(
				deg2rad(0.0f),
				deg2rad(0.0f),
				deg2rad(90.0f)
			)
		));
		bs2.setLinearVelocity(Vector4(-7.0f, -8.0f, -9.0f, 0.0f));
		bs2.setAngularVelocity(Vector4(0.0f, 0.0f, 1.0f, 0.0f));

		net::StateTemplate st;
		st.declare(new net::BodyStateTemplate());

		net::State s0;
		s0.pack< net::BodyStateValue >(bs0);

		net::State s1;
		s1.pack< net::BodyStateValue >(bs1);

		net::State s2;
		s2.pack< net::BodyStateValue >(bs2);

		Ref< const net::State > S0 = transfer(&st, &s0);
		Ref< const net::State > S1 = transfer(&st, &s1);
		Ref< const net::State > S2 = transfer(&st, &s2);

		Ref< const net::State > r0 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 0.0f);
		Ref< const net::State > r1 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 0.5f);
		Ref< const net::State > r2 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 1.0f);
		Ref< const net::State > r3 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 1.5f);
		Ref< const net::State > r4 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 2.0f);

		physics::BodyState bsv0 = r0->getValue< net::BodyStateValue >(0);
		CASE_ASSERT_COMPARE(bsv0.getTransform(), bs0.getTransform(), compareTransformEqual);
		CASE_ASSERT_COMPARE(bsv0.getLinearVelocity(), bs0.getLinearVelocity(), compareVectorEqual);
		CASE_ASSERT_COMPARE(bsv0.getAngularVelocity(), bs0.getAngularVelocity(), compareVectorEqual);

		physics::BodyState bsv1 = r1->getValue< net::BodyStateValue >(0);
		CASE_ASSERT_COMPARE(bsv1.getTransform(), bs1.getTransform(), compareTransformEqual);
		CASE_ASSERT_COMPARE(bsv1.getLinearVelocity(), bs1.getLinearVelocity(), compareVectorEqual);
		CASE_ASSERT_COMPARE(bsv1.getAngularVelocity(), bs1.getAngularVelocity(), compareVectorEqual);

		physics::BodyState bsv2 = r2->getValue< net::BodyStateValue >(0);
		CASE_ASSERT_COMPARE(bsv2.getTransform(), bs2.getTransform(), compareTransformEqual);
		CASE_ASSERT_COMPARE(bsv2.getLinearVelocity(), bs2.getLinearVelocity(), compareVectorEqual);
		CASE_ASSERT_COMPARE(bsv2.getAngularVelocity(), bs2.getAngularVelocity(), compareVectorEqual);



	}
}

}
