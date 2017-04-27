/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Net/Replication/State/IValue.h"
#include "Net/Replication/State/IValueTemplate.h"
#include "Net/Replication/State/State.h"
#include "Net/Replication/State/StateTemplate.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.StateTemplate", StateTemplate, Object)

void StateTemplate::declare(const IValueTemplate* value)
{
	if (value)
		m_valueTemplates.push_back(value);
}

bool StateTemplate::match(const State* S) const
{
	if (!S)
		return false;

	const RefArray< const IValue >& V = S->getValues();
	if (V.size() != m_valueTemplates.size())
		return false;

	for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
	{
		const IValueTemplate* valueTemplate = m_valueTemplates[i];
		T_ASSERT (valueTemplate);

		const TypeInfo& valueType = valueTemplate->getValueType();
		if (!is_type_a(valueType, type_of(V[i])))
			return false;
	}

	return true;
}

bool StateTemplate::critical(const State* Sn1, const State* S0) const
{
	if (!Sn1 || !S0)
		return bool(S0 != 0);

	const RefArray< const IValue >& Vn1 = Sn1->getValues();
	const RefArray< const IValue >& V0 = S0->getValues();

	if (Vn1.size() != m_valueTemplates.size() || V0.size() != m_valueTemplates.size())
		return false;

	for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
	{
		const IValueTemplate* valueTemplate = m_valueTemplates[i];
		T_ASSERT (valueTemplate);

		if (valueTemplate->threshold(Vn1[i], V0[i]))
			return true;
	}

	return false;
}

Ref< const State > StateTemplate::extrapolate(const State* Sn2, float Tn2, const State* Sn1, float Tn1, const State* S0, float T0, float T) const
{
	RefArray< const IValue > Vr(m_valueTemplates.size());

	// Ensure times are valid.
	if (isNanOrInfinite(Tn2) || isNanOrInfinite(Tn1) || isNanOrInfinite(T0) || isNanOrInfinite(T))
	{
		log::error << L"Invalid state time(s); either NaN or Inf." << Endl;
		return 0;
	}

	if (Sn2 && Sn1 && S0)
	{
		const RefArray< const IValue >& Vn2 = Sn2->getValues();
		const RefArray< const IValue >& Vn1 = Sn1->getValues();
		const RefArray< const IValue >& V0 = S0->getValues();

		if (
			Vn2.size() != m_valueTemplates.size() ||
			Vn1.size() != m_valueTemplates.size() ||
			V0.size() != m_valueTemplates.size()
		)
		{
			log::error << L"Number of state values mismatch template definition; #Vn2 " << int32_t(Vn2.size()) << L", #Vn1 " << int32_t(Vn1.size()) << L", #V0 " << int32_t(V0.size()) << L"." << Endl;
			return 0;
		}

		if (Tn2 > Tn1 - FUZZY_EPSILON)
		{
			log::error << L"Difference between Tn2 and Tn1 too small." << Endl;
			return 0;
		}
		if (Tn1 > T0 - FUZZY_EPSILON)
		{
			log::error << L"Difference between Tn1 and T0 too small." << Endl;
			return 0;
		}

		for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
		{
			const IValueTemplate* valueTemplate = m_valueTemplates[i];
			T_ASSERT (valueTemplate);

			const TypeInfo& valueType = valueTemplate->getValueType();
			if (
				!is_type_a(valueType, type_of(Vn2[i])) ||
				!is_type_a(valueType, type_of(Vn1[i])) ||
				!is_type_a(valueType, type_of(V0[i]))
			)
			{
				log::error << L"Value types mismatch template definition" << Endl;
				log::error << L"\tDefinition \"" << valueType.getName() << L"\"" << Endl;
				log::error << L"\tVn2 \"" << type_of(Vn2[i]).getName() << L"\"" << Endl;
				log::error << L"\tVn1 \"" << type_of(Vn1[i]).getName() << L"\"" << Endl;
				log::error << L"\tV0 \"" << type_of(V0[i]).getName() << L"\"" << Endl;
				return 0;
			}

			Vr[i] = valueTemplate->extrapolate(Vn2[i], Tn2, Vn1[i], Tn1, V0[i], T0, T);
			T_ASSERT (Vr[i]);
		}
	}
	else if (Sn1 && S0)
	{
		const RefArray< const IValue >& Vn1 = Sn1->getValues();
		const RefArray< const IValue >& V0 = S0->getValues();

		if (
			Vn1.size() != m_valueTemplates.size() ||
			V0.size() != m_valueTemplates.size()
		)
		{
			log::error << L"Number of state values mismatch template definition; #Vn1 " << int32_t(Vn1.size()) << L", #V0 " << int32_t(V0.size()) << L"." << Endl;
			return 0;
		}

		if (Tn1 > T0 - FUZZY_EPSILON)
		{
			log::error << L"Difference between Tn1 and T0 too small." << Endl;
			return 0;
		}

		for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
		{
			const IValueTemplate* valueTemplate = m_valueTemplates[i];
			T_ASSERT (valueTemplate);

			const TypeInfo& valueType = valueTemplate->getValueType();
			if (
				!is_type_a(valueType, type_of(Vn1[i])) ||
				!is_type_a(valueType, type_of(V0[i]))
			)
			{
				log::error << L"Value types mismatch template definition" << Endl;
				log::error << L"\tDefinition \"" << valueType.getName() << L"\"" << Endl;
				log::error << L"\tVn1 \"" << type_of(Vn1[i]).getName() << L"\"" << Endl;
				log::error << L"\tV0 \"" << type_of(V0[i]).getName() << L"\"" << Endl;
				return 0;
			}

			Vr[i] = valueTemplate->extrapolate(Vn1[i], 0.0f, Vn1[i], Tn1, V0[i], T0, T);
			T_ASSERT (Vr[i]);
		}
	}
	else if (S0)
	{
		const RefArray< const IValue >& V0 = S0->getValues();

		if (V0.size() != m_valueTemplates.size())
		{
			log::error << L"Number of state values mismatch template definition; #V0 " << int32_t(V0.size()) << Endl;
			return 0;
		}

		for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
		{
			const IValueTemplate* valueTemplate = m_valueTemplates[i];
			T_ASSERT (valueTemplate);

			const TypeInfo& valueType = valueTemplate->getValueType();
			if (!is_type_a(valueType, type_of(V0[i])))
			{
				log::error << L"Value types mismatch template definition" << Endl;
				log::error << L"\tDefinition \"" << valueType.getName() << L"\"" << Endl;
				log::error << L"\tV0 \"" << type_of(V0[i]).getName() << L"\"" << Endl;
				return 0;
			}

			Vr[i] = V0[i];
			T_ASSERT (Vr[i]);
		}
	}
	else
		return 0;

	return new State(Vr);
}

uint32_t StateTemplate::pack(const State* S, void* buffer, uint32_t bufferSize) const
{
	T_FATAL_ASSERT (S);

	// Number of values of state must match template.
	const RefArray< const IValue >& V = S->getValues();
	if (V.size() != m_valueTemplates.size())
	{
		log::error << L"State values mismatch template definition." << Endl;
		return 0;
	}

	// Ensure all values have correct type.
	uint32_t maxPackedSize = 0;
	for (uint32_t i = 0; i < m_valueTemplates.size(); ++i)
	{
		const IValueTemplate* valueTemplate = m_valueTemplates[i];
		T_ASSERT (valueTemplate);

		const TypeInfo& valueType = valueTemplate->getValueType();
		if (!is_type_a(valueType, type_of(V[i])))
		{
			log::error << L"Value types mismatch template definition" << Endl;
			log::error << L"\tDefinition \"" << valueType.getName() << L"\"" << Endl;
			log::error << L"\tV \"" << type_of(V[i]).getName() << L"\"" << Endl;
			return 0;
		}

		maxPackedSize += valueTemplate->getMaxPackedDataSize();
	}

	// Ensure all values fit within output buffer.
	if ((maxPackedSize + 7) / 8 > bufferSize)
	{
		log::error << L"Not enough size in packed buffer to pack all values; state discarded." << Endl;
		return 0;
	}

	// Pack all values into buffer.
	MemoryStream stream(buffer, bufferSize, false, true);
	BitWriter writer(&stream);

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

	// Must have read all data from buffer.
	if (stream.available() > 0)
	{
		log::error << L"Not all state data has been unpacked; entire state discarded." << Endl;
		return 0;
	}

	return new State(V);
}

	}
}
