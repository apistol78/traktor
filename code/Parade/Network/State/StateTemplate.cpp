#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Io/MemoryStream.h"
#include "Parade/Network/State/IValueTemplate.h"
#include "Parade/Network/State/State.h"
#include "Parade/Network/State/StateTemplate.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

const float c_maxExtrapolationDelta = 2.0f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.StateTemplate", StateTemplate, Object)

void StateTemplate::declare(const IValueTemplate* value)
{
	m_valueTemplates.push_back(value);
}

Ref< const State > StateTemplate::extrapolate(const State* Sn2, float Tn2, const State* Sn1, float Tn1, const State* S0, float T0, const State* S, float T) const
{
	if (T > T0 + c_maxExtrapolationDelta)
		return 0;

	if (Sn2 && Sn1 && S0 && S)
	{
		const RefArray< const IValue >& Vn2 = Sn2->getValues();
		const RefArray< const IValue >& Vn1 = Sn1->getValues();
		const RefArray< const IValue >& V0 = S0->getValues();
		const RefArray< const IValue >& V = S->getValues();

		RefArray< const IValue > Vr(m_valueTemplates.size());
		for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
		{
			const IValueTemplate* valueTemplate = m_valueTemplates[i];
			T_ASSERT (valueTemplate);

			Vr[i] = valueTemplate->extrapolate(Vn2[i], Tn2, Vn1[i], Tn1, V0[i], T0, V[i], T);
		}

		return new State(Vr);
	}
	else if (Sn2 && Sn1 && S0)
	{
		const RefArray< const IValue >& Vn2 = Sn2->getValues();
		const RefArray< const IValue >& Vn1 = Sn1->getValues();
		const RefArray< const IValue >& V0 = S0->getValues();

		RefArray< const IValue > Vr(m_valueTemplates.size());
		for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
		{
			const IValueTemplate* valueTemplate = m_valueTemplates[i];
			T_ASSERT (valueTemplate);

			Vr[i] = valueTemplate->extrapolate(Vn2[i], Tn2, Vn1[i], Tn1, V0[i], T0, 0, T);
		}

		return new State(Vr);
	}
	else if (Sn1 && S0)
	{
		const RefArray< const IValue >& Vn1 = Sn1->getValues();
		const RefArray< const IValue >& V0 = S0->getValues();

		RefArray< const IValue > V(m_valueTemplates.size());
		for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
		{
			const IValueTemplate* valueTemplate = m_valueTemplates[i];
			T_ASSERT (valueTemplate);

			V[i] = valueTemplate->extrapolate(0, 0.0f, Vn1[i], Tn1, V0[i], T0, 0, T);
		}

		return new State(V);
	}
	else if (S0)
	{
		const RefArray< const IValue >& V0 = S0->getValues();
		return new State(V0);
	}
	else
		return 0;
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

	}
}
