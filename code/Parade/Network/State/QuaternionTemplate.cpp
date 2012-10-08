#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Parade/Network/State/QuaternionValue.h"
#include "Parade/Network/State/QuaternionTemplate.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.QuaternionTemplate", QuaternionTemplate, IValueTemplate)

void QuaternionTemplate::pack(BitWriter& writer, const IValue* V) const
{
	Quaternion v = *checked_type_cast< const QuaternionValue* >(V);
	
	float T_MATH_ALIGN16 e[4];
	v.e.storeAligned(e);

	writer.writeUnsigned(32, *(uint32_t*)&e[0]);
	writer.writeUnsigned(32, *(uint32_t*)&e[1]);
	writer.writeUnsigned(32, *(uint32_t*)&e[2]);
	writer.writeUnsigned(32, *(uint32_t*)&e[3]);
}

Ref< const IValue > QuaternionTemplate::unpack(BitReader& reader) const
{
	uint32_t ua = reader.readUnsigned(32);
	uint32_t ub = reader.readUnsigned(32);
	uint32_t uc = reader.readUnsigned(32);
	uint32_t ud = reader.readUnsigned(32);

	return new QuaternionValue(Quaternion(
		*(float*)&ua,
		*(float*)&ub,
		*(float*)&uc,
		*(float*)&ud
	));
}

Ref< const IValue > QuaternionTemplate::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, const IValue* V, float T) const
{
	Quaternion fn1 = *checked_type_cast< const QuaternionValue* >(Vn1);
	Quaternion f0 = *checked_type_cast< const QuaternionValue* >(V0);
	Scalar k = Scalar((T - Tn1) / (T0 - Tn1));
	return new QuaternionValue(f0 * slerp(Quaternion::identity(), fn1.inverse() * f0, k).normalized());
}

	}
}
