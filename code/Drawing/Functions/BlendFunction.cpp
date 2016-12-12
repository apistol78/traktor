#include "Drawing/Functions/BlendFunction.h"

namespace traktor
{
	namespace drawing
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.BlendFunction", BlendFunction, ITransferFunction)
	
BlendFunction::BlendFunction(Factor sourceFactor, Factor destinationFactor, Operation operation)
:	m_sourceFactor(sourceFactor)
,	m_destinationFactor(destinationFactor)
,	m_operation(operation)
{
}

void BlendFunction::evaluate(const Color4f& in, Color4f& out) const
{
	Color4f src, dst;

	switch (m_sourceFactor)
	{
	case BfOne:
		src = in;
		break;

	case BfZero:
		src = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
		break;

	case BfSourceColor:
		src = in * in;
		break;

	case BfOneMinusSourceColor:
		src = in * (Color4f(1.0f, 1.0f, 1.0f, 1.0f) - in);
		break;

	case BfDestinationColor:
		src = in * out;
		break;

	case BfOneMinusDestinationColor:
		src = in * (Color4f(1.0f, 1.0f, 1.0f, 1.0f) - out);
		break;

	case BfSourceAlpha:
		src = in * in.getAlpha();
		break;

	case BfOneMinusSourceAlpha:
		src = in * (Scalar(1.0f) - in.getAlpha());
		break;

	case BfDestinationAlpha:
		src = in * out.getAlpha();
		break;

	case BfOneMinusDestinationAlpha:
		src = in * (Scalar(1.0f) - out.getAlpha());
		break;
	}

	switch (m_destinationFactor)
	{
	case BfOne:
		dst = out;
		break;

	case BfZero:
		dst = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
		break;

	case BfSourceColor:
		dst = out * in;
		break;

	case BfOneMinusSourceColor:
		dst = out * (Color4f(1.0f, 1.0f, 1.0f, 1.0f) - in);
		break;

	case BfDestinationColor:
		dst = out * out;
		break;

	case BfOneMinusDestinationColor:
		dst = out * (Color4f(1.0f, 1.0f, 1.0f, 1.0f) - out);
		break;

	case BfSourceAlpha:
		dst = out * in.getAlpha();
		break;

	case BfOneMinusSourceAlpha:
		dst = out * (Scalar(1.0f) - in.getAlpha());
		break;

	case BfDestinationAlpha:
		dst = out * out.getAlpha();
		break;

	case BfOneMinusDestinationAlpha:
		dst = out * (Scalar(1.0f) - out.getAlpha());
		break;
	}

	switch (m_operation)
	{
	case BoAdd:
		out = src + dst;
		break;

	case BoSubtract:
		out = src - dst;
		break;

	case BoReverseSubtract:
		out = dst - src;
		break;

	case BoMin:
		out = Color4f(min((Vector4)src, (Vector4)dst));
		break;

	case BoMax:
		out = Color4f(max((Vector4)src, (Vector4)dst));
		break;
	}
}

	}
}
