#include "Core/Math/Const.h"
#include "Model/Model.h"
#include "Model/Operations/Quantize.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Quantize", Quantize, IModelOperation)

Quantize::Quantize(float step)
:	m_step(step, step, step, 1.0f)
{
}

Quantize::Quantize(const Vector4& step)
:	m_step(step.xyz1())
{
}

bool Quantize::apply(Model& model) const
{
	float T_MATH_ALIGN16 e[4];
	AlignedVector< Vector4 > positions = model.getPositions();
	for (AlignedVector< Vector4 >::iterator i = positions.begin(); i != positions.end(); ++i)
	{
		(*i / m_step).storeAligned(e);

		e[0] = std::floor(e[0]);
		e[1] = std::floor(e[1]);
		e[2] = std::floor(e[2]);

		*i = Vector4::loadAligned(e) * m_step;
	}
	model.setPositions(positions);
	return true;
}

	}
}
