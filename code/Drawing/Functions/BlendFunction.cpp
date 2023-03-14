/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Drawing/Functions/BlendFunction.h"

namespace traktor::drawing
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
	case One:
		src = in;
		break;

	case Zero:
		src = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
		break;

	case SourceColor:
		src = in * in;
		break;

	case OneMinusSourceColor:
		src = in * (Color4f(1.0f, 1.0f, 1.0f, 1.0f) - in);
		break;

	case DestinationColor:
		src = in * out;
		break;

	case OneMinusDestinationColor:
		src = in * (Color4f(1.0f, 1.0f, 1.0f, 1.0f) - out);
		break;

	case SourceAlpha:
		src = in * in.getAlpha();
		break;

	case OneMinusSourceAlpha:
		src = in * (Scalar(1.0f) - in.getAlpha());
		break;

	case DestinationAlpha:
		src = in * out.getAlpha();
		break;

	case OneMinusDestinationAlpha:
		src = in * (Scalar(1.0f) - out.getAlpha());
		break;
	}

	switch (m_destinationFactor)
	{
	case One:
		dst = out;
		break;

	case Zero:
		dst = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
		break;

	case SourceColor:
		dst = out * in;
		break;

	case OneMinusSourceColor:
		dst = out * (Color4f(1.0f, 1.0f, 1.0f, 1.0f) - in);
		break;

	case DestinationColor:
		dst = out * out;
		break;

	case OneMinusDestinationColor:
		dst = out * (Color4f(1.0f, 1.0f, 1.0f, 1.0f) - out);
		break;

	case SourceAlpha:
		dst = out * in.getAlpha();
		break;

	case OneMinusSourceAlpha:
		dst = out * (Scalar(1.0f) - in.getAlpha());
		break;

	case DestinationAlpha:
		dst = out * out.getAlpha();
		break;

	case OneMinusDestinationAlpha:
		dst = out * (Scalar(1.0f) - out.getAlpha());
		break;
	}

	switch (m_operation)
	{
	case Add:
		out = src + dst;
		break;

	case Subtract:
		out = src - dst;
		break;

	case ReverseSubtract:
		out = dst - src;
		break;

	case Min:
		out = Color4f(min((Vector4)src, (Vector4)dst));
		break;

	case Max:
		out = Color4f(max((Vector4)src, (Vector4)dst));
		break;
	}
}

}
