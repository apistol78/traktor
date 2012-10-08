#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Parade/Network/State/Vector4Value.h"
#include "Parade/Network/State/Vector4Template.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.Vector4Template", Vector4Template, IValueTemplate)

Vector4Template::Vector4Template()
:	m_w(0.0f)
,	m_immutableW(false)
{
}

Vector4Template::Vector4Template(float w)
:	m_w(w)
,	m_immutableW(true)
{
}

void Vector4Template::pack(BitWriter& writer, const IValue* V) const
{
	Vector4 v = *checked_type_cast< const Vector4Value* >(V);

	float T_MATH_ALIGN16 e[4];
	v.storeAligned(e);

	writer.writeUnsigned(32, *(uint32_t*)&e[0]);
	writer.writeUnsigned(32, *(uint32_t*)&e[1]);
	writer.writeUnsigned(32, *(uint32_t*)&e[2]);

	if (!m_immutableW)
		writer.writeUnsigned(32, *(uint32_t*)&e[3]);
}

Ref< const IValue > Vector4Template::unpack(BitReader& reader) const
{
	uint32_t ux = reader.readUnsigned(32);
	uint32_t uy = reader.readUnsigned(32);
	uint32_t uz = reader.readUnsigned(32);

	if (!m_immutableW)
	{
		uint32_t uw = reader.readUnsigned(32);
		return new Vector4Value(Vector4(
			*(float*)&ux,
			*(float*)&uy,
			*(float*)&uz,
			*(float*)&uw
		));
	}
	else
	{
		return new Vector4Value(Vector4(
			*(float*)&ux,
			*(float*)&uy,
			*(float*)&uz,
			m_w
		));
	}
}

Ref< const IValue > Vector4Template::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, const IValue* V, float T) const
{
	Vector4 fn1 = *checked_type_cast< const Vector4Value* >(Vn1);
	Vector4 f0 = *checked_type_cast< const Vector4Value* >(V0);
	float k = (T - Tn1) / (T0 - Tn1);
	return new Vector4Value(f0 + (fn1 - f0) * Scalar(k));
}

	}
}
