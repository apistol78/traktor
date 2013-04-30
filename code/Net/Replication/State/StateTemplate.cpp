#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Net/Replication/State/IValueTemplate.h"
#include "Net/Replication/State/State.h"
#include "Net/Replication/State/StateTemplate.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

const float c_maxExtrapolationDelta = 2.0f;
const float c_equalErrorThreshold = 0.001f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.StateTemplate", StateTemplate, Object)

void StateTemplate::declare(const IValueTemplate* value)
{
	m_valueTemplates.push_back(value);
}

Ref< const State > StateTemplate::extrapolate(const State* Sn2, float Tn2, const State* Sn1, float Tn1, const State* S0, float T0, const State* S, float T) const
{
	if (T > T0 + c_maxExtrapolationDelta || !S)
		return 0;

	const RefArray< const IValue >& V = S->getValues();
	RefArray< const IValue > Vr(m_valueTemplates.size());

	if (Sn2 && Sn1 && S0)
	{
		const RefArray< const IValue >& Vn2 = Sn2->getValues();
		const RefArray< const IValue >& Vn1 = Sn1->getValues();
		const RefArray< const IValue >& V0 = S0->getValues();

		for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
		{
			const IValueTemplate* valueTemplate = m_valueTemplates[i];
			T_ASSERT (valueTemplate);

			Vr[i] = valueTemplate->extrapolate(Vn2[i], Tn2, Vn1[i], Tn1, V0[i], T0, V[i], T);
		}
	}
	else if (Sn1 && S0)
	{
		const RefArray< const IValue >& Vn1 = Sn1->getValues();
		const RefArray< const IValue >& V0 = S0->getValues();

		for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
		{
			const IValueTemplate* valueTemplate = m_valueTemplates[i];
			T_ASSERT (valueTemplate);

			Vr[i] = valueTemplate->extrapolate(0, 0.0f, Vn1[i], Tn1, V0[i], T0, V[i], T);
		}
	}
	else if (S0)
	{
		const RefArray< const IValue >& V0 = S0->getValues();

		for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
		{
			const IValueTemplate* valueTemplate = m_valueTemplates[i];
			T_ASSERT (valueTemplate);

			Vr[i] = V0[i];
		}
	}

	return new State(Vr);
}

uint32_t StateTemplate::pack(const State* S, void* buffer, uint32_t bufferSize) const
{
	MemoryStream stream(buffer, bufferSize, false, true);
	BitWriter writer(&stream);

	const RefArray< const IValue >& V = S->getValues();
	for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
	{
		const IValueTemplate* valueTemplate = m_valueTemplates[i];
		T_ASSERT (valueTemplate);

		valueTemplate->pack(writer, V[i]);
	}

	writer.flush();
	return stream.tell();
}

uint32_t StateTemplate::pack(const State* Sn1, const State* S, void* buffer, uint32_t bufferSize) const
{
	MemoryStream stream(buffer, bufferSize, false, true);
	BitWriter writer(&stream);

	const RefArray< const IValue >& Vn1 = Sn1->getValues();
	const RefArray< const IValue >& V = S->getValues();

	for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
	{
		const IValueTemplate* valueTemplate = m_valueTemplates[i];
		T_ASSERT (valueTemplate);

		if (valueTemplate->error(Vn1[i], V[i]) > c_equalErrorThreshold)
		{
			writer.writeBit(1);
			valueTemplate->pack(writer, V[i]);
		}
		else
			writer.writeBit(0);
	}

	writer.flush();
	return stream.tell();
}

Ref< const State > StateTemplate::unpack(const void* buffer, uint32_t bufferSize) const
{
	MemoryStream stream(buffer, bufferSize);
	BitReader reader(&stream);

	RefArray< const IValue > V(m_valueTemplates.size());
	for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
	{
		const IValueTemplate* valueTemplate = m_valueTemplates[i];
		T_ASSERT (valueTemplate);

		if ((V[i] = valueTemplate->unpack(reader)) == 0)
			return 0;
	}

	return new State(V);
}

Ref< const State > StateTemplate::unpack(const State* Sn1, const void* buffer, uint32_t bufferSize) const
{
	MemoryStream stream(buffer, bufferSize);
	BitReader reader(&stream);

	const RefArray< const IValue >& Vn1 = Sn1->getValues();
	RefArray< const IValue > V(m_valueTemplates.size());

	for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
	{
		const IValueTemplate* valueTemplate = m_valueTemplates[i];
		T_ASSERT (valueTemplate);

		if (reader.readBit())
		{
			if ((V[i] = valueTemplate->unpack(reader)) == 0)
				return 0;
		}
		else
			V[i] = Vn1[i];
	}

	return new State(V);
}

float StateTemplate::error(const State* Sl, const State* Sr) const
{
	const RefArray< const IValue >& Vl = Sl->getValues();
	const RefArray< const IValue >& Vr = Sr->getValues();

	float E = 0.0f;
	for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
	{
		const IValueTemplate* valueTemplate = m_valueTemplates[i];
		T_ASSERT (valueTemplate);

		float e = valueTemplate->error(Vl[i], Vr[i]);
		E = max(e, E);
	}

	return E;
}

	}
}
