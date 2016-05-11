#include <cmath>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Matrix44.h"
#include "Render/Sw/Core/InterpreterFixed.h"
#include "Render/Sw/Core/IntrProgram.h"
#include "Render/Sw/Core/Sampler.h"

#undef min
#undef max

namespace traktor
{
	namespace render
	{
		namespace
		{

struct InternalImage
{
	AlignedVector< Instruction > instructions;
	AlignedVector< Fix4 > constants;
};

inline fp32_t clamp(fp32_t v, fp32_t min, fp32_t max)
{
	if (v < min)
		return min;
	if (v > max)
		return max;
	return v;
}

inline fp32_t fraction(fp32_t v)
{
	if (v >= 0)
		return v & 0x0000ffff;
	else
		return -((-v) & 0x0000ffff);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.InterpreterFixed", InterpreterFixed, Processor)

fp32_t InterpreterFixed::ms_trig[256];

InterpreterFixed::InterpreterFixed()
{
	for (uint32_t i = 0; i < 256; ++i)
		ms_trig[i] = toFixed((float)sin(i * PI * 2.0 / 255.0));
}

Processor::image_t InterpreterFixed::compile(const IntrProgram& program) const
{
	InternalImage* img = new InternalImage();
	
	img->instructions = program.getInstructions();
	
	img->constants.resize(program.getConstants().size());
	for (uint32_t i = 0; i < uint32_t(program.getConstants().size()); ++i)
		img->constants[i] = toFixed(program.getConstant(i));

	return img;
}

void InterpreterFixed::destroy(image_t image) const
{
	InternalImage* img = reinterpret_cast< InternalImage* >(image);
	delete img;
}

bool InterpreterFixed::execute(
	const image_t image,
	int32_t instance,
	const Vector4* inUniforms,
	const Vector4* inVaryings,
	const Vector4& targetSize,
	const Vector4& fragmentPosition,
	const Ref< AbstractSampler >* inSamplers,
	Vector4* outVaryings
) const
{
	InternalImage* img = reinterpret_cast< InternalImage* >(image);
	T_ASSERT (img);

	Fix4 R[256];

	const AlignedVector< Instruction >& instructions = img->instructions;
	for (AlignedVector< Instruction >::const_iterator i = instructions.begin(); i != instructions.end(); ++i)
	{
		Fix4& dest = R[i->dest];
		switch (i->op)
		{
		case OpNop:
			break;

		case OpFetchConstant:
			dest = img->constants[i->src[0]];
			break;

		case OpFetchInstance:
			dest = toFixed(Vector4(
				float(instance),
				float(instance),
				float(instance),
				float(instance)
			));
			break;

		case OpFetchTargetSize:
			dest = toFixed(targetSize);
			break;

		case OpFetchFragmentPosition:
			dest = toFixed(fragmentPosition);
			break;

		case OpFetchUniform:
			{
				for (uint32_t j = 0; j < i->src[1]; ++j)
					R[i->dest + j] = toFixed(inUniforms[i->src[0] + j]);
			}
			break;

		case OpFetchIndexedUniform:
			{
				uint32_t index = toInt(R[i->src[2]].x) * i->src[1];
				for (uint32_t j = 0; j < i->src[1]; ++j)
					R[i->dest + j] = toFixed(inUniforms[i->src[0] + j + index]);
			}
			break;

		case OpFetchVarying:
			dest = toFixed(inVaryings[i->src[0]]);
			break;

		case OpStoreVarying:
			outVaryings[i->dest] = toFloat(R[i->src[0]]);
			break;

		case OpMove:
			dest = R[i->src[0]];
			break;

		case OpAbs:
			dest.x = abs(R[i->src[0]].x);
			dest.y = abs(R[i->src[0]].y);
			dest.z = abs(R[i->src[0]].z);
			dest.w = abs(R[i->src[0]].w);
			break;

		case OpIncrement:
			dest.x += toFixed(1);
			dest.y += toFixed(1);
			dest.z += toFixed(1);
			dest.w += toFixed(1);
			break;

		case OpDecrement:
			dest.x -= toFixed(1);
			dest.y -= toFixed(1);
			dest.z -= toFixed(1);
			dest.w -= toFixed(1);
			break;

		case OpAdd:
			dest = R[i->src[0]] + R[i->src[1]];
			break;

		case OpDiv:
			dest = R[i->src[0]] / R[i->src[1]];
			break;

		case OpMul:
			dest = R[i->src[0]] * R[i->src[1]];
			break;

		case OpMulAdd:
			dest = R[i->src[0]] * R[i->src[1]] + R[i->src[2]];
			break;

		//case OpLog:
		//	break;

		//case OpLog2:
		//	break;

		//case OpLog10:
		//	break;

		//case OpExp:
		//	break;

		case OpFraction:
			dest.x = fraction(R[i->src[0]].x);
			dest.y = fraction(R[i->src[1]].y);
			dest.z = fraction(R[i->src[2]].z);
			dest.w = fraction(R[i->src[3]].w);
			break;

		case OpNeg:
			dest = -R[i->src[0]];
			break;

		case OpPow:
			dest.x = fixedPow(R[i->src[0]].x, R[i->src[1]].x);
			dest.y = fixedPow(R[i->src[0]].y, R[i->src[1]].y);
			dest.z = fixedPow(R[i->src[0]].z, R[i->src[1]].z);
			dest.w = fixedPow(R[i->src[0]].w, R[i->src[1]].w);
			break;

		case OpSqrt:
			dest.x = fixedSqrt(R[i->src[0]].x);
			dest.y = fixedSqrt(R[i->src[0]].y);
			dest.z = fixedSqrt(R[i->src[0]].z);
			dest.w = fixedSqrt(R[i->src[0]].w);
			break;

		case OpRecipSqrt:
			break;

		case OpSub:
			dest = R[i->src[0]] - R[i->src[1]];
			break;

		case OpAcos:
			dest.x = fixedArcCos(R[i->src[0]].x);
			dest.y = fixedArcCos(R[i->src[0]].y);
			dest.z = fixedArcCos(R[i->src[0]].z);
			dest.w = fixedArcCos(R[i->src[0]].w);
			break;

		case OpAtan:
			dest.x = fixedArcTan(R[i->src[0]].x);
			dest.y = fixedArcTan(R[i->src[0]].y);
			dest.z = fixedArcTan(R[i->src[0]].z);
			dest.w = fixedArcTan(R[i->src[0]].w);
			break;

		case OpCos:
			dest.x = fixedCos(R[i->src[0]].x);
			dest.y = fixedCos(R[i->src[0]].y);
			dest.z = fixedCos(R[i->src[0]].z);
			dest.w = fixedCos(R[i->src[0]].w);
			break;

		case OpSin:
			dest.x = fixedSin(R[i->src[0]].x);
			dest.y = fixedSin(R[i->src[0]].y);
			dest.z = fixedSin(R[i->src[0]].z);
			dest.w = fixedSin(R[i->src[0]].w);
			break;

		case OpTan:
			dest.x = fixedTan(R[i->src[0]].x);
			dest.y = fixedTan(R[i->src[0]].y);
			dest.z = fixedTan(R[i->src[0]].z);
			dest.w = fixedTan(R[i->src[0]].w);
			break;

		case OpCross:
			dest = cross(R[i->src[0]], R[i->src[1]]);
			break;

		case OpDot3:
			{
				fp32_t a = dot3(R[i->src[0]], R[i->src[1]]);
				dest.set(a, a, a, a);
			}
			break;

		case OpDot4:
			{
				fp32_t a = dot4(R[i->src[0]], R[i->src[1]]);
				dest.set(a, a, a, a);
			}
			break;

		case OpLength:
			{
				fp32_t ln2 = dot4(R[i->src[0]], R[i->src[0]]);
				fp32_t ln = fixedSqrt(ln2);
				dest.set(ln, ln, ln, ln);
			}
			break;

		case OpNormalize:
			{
				fp32_t ln2 = dot4(R[i->src[0]], R[i->src[0]]);
				fp32_t ln = fixedSqrt(ln2);
				dest = R[i->src[0]] / ln;
			}
			break;

		case OpTransform:
			{
				dest =
					R[i->src[1] + 0] * R[i->src[0]].x +
					R[i->src[1] + 1] * R[i->src[0]].y +
					R[i->src[1] + 2] * R[i->src[0]].z +
					R[i->src[1] + 3] * R[i->src[0]].w;
			}
			break;

		case OpTranspose:
			{
				Fix4 s1 = R[i->src[0] + 0];
				Fix4 s2 = R[i->src[0] + 1];
				Fix4 s3 = R[i->src[0] + 2];
				Fix4 s4 = R[i->src[0] + 3];

				Fix4& d1 = R[i->dest + 0];
				Fix4& d2 = R[i->dest + 1];
				Fix4& d3 = R[i->dest + 2];
				Fix4& d4 = R[i->dest + 3];

				d1.set(s1.x, s2.x, s3.x, s4.x);
				d2.set(s1.y, s2.y, s3.y, s4.y);
				d3.set(s1.z, s2.z, s3.z, s4.z);
				d4.set(s1.w, s2.w, s3.w, s4.w);
			}
			break;

		case OpClamp:
			{
				const Fix4& mn = img->constants[i->src[1]];
				const Fix4& mx = img->constants[i->src[2]];

				dest.x = clamp(R[i->src[0]].x, mn.x, mx.x);
				dest.y = clamp(R[i->src[0]].y, mn.y, mx.y);
				dest.z = clamp(R[i->src[0]].z, mn.z, mx.z);
				dest.w = clamp(R[i->src[0]].w, mn.w, mx.w);
			}
			break;

		case OpTrunc:
			break;

		case OpRound:
			break;

		case OpLerp:
			{
				fp32_t b = R[i->src[0]].x;
				fp32_t a = c_one - b;
				dest = R[i->src[1]] * a + R[i->src[2]] * b;
			}
			break;

		case OpMixIn:
			{
				fp32_t x = R[i->src[0]].x;
				fp32_t y = R[i->src[1]].x;
				fp32_t z = R[i->src[2]].x;
				fp32_t w = R[i->src[3]].x;
				dest.set(x, y, z, w);
			}
			break;

		case OpMin:
			dest.x = std::min(R[i->src[0]].x, R[i->src[1]].x);
			dest.y = std::min(R[i->src[0]].y, R[i->src[1]].y);
			dest.z = std::min(R[i->src[0]].z, R[i->src[1]].z);
			dest.w = std::min(R[i->src[0]].w, R[i->src[1]].w);
			break;

		case OpMax:
			dest.x = std::max(R[i->src[0]].x, R[i->src[1]].x);
			dest.y = std::max(R[i->src[0]].y, R[i->src[1]].y);
			dest.z = std::max(R[i->src[0]].z, R[i->src[1]].z);
			dest.w = std::max(R[i->src[0]].w, R[i->src[1]].w);
			break;

		case OpStep:
			{
				fp32_t x = R[i->src[0]].x >= R[i->src[1]].x ? toFixed(1.0f) : toFixed(0.0f);
				fp32_t y = R[i->src[0]].y >= R[i->src[1]].y ? toFixed(1.0f) : toFixed(0.0f);
				fp32_t z = R[i->src[0]].z >= R[i->src[1]].z ? toFixed(1.0f) : toFixed(0.0f);
				fp32_t w = R[i->src[0]].w >= R[i->src[1]].w ? toFixed(1.0f) : toFixed(0.0f);
				dest.set(x, y, z, w);
			}
			break;

		case OpSign:
			{
				fp32_t x = (R[i->src[0]].x >= 0 ? toFixed(1.0f) : toFixed(-1.0f));
				fp32_t y = (R[i->src[0]].y >= 0 ? toFixed(1.0f) : toFixed(-1.0f));
				fp32_t z = (R[i->src[0]].z >= 0 ? toFixed(1.0f) : toFixed(-1.0f));
				fp32_t w = (R[i->src[0]].w >= 0 ? toFixed(1.0f) : toFixed(-1.0f));
				dest.set(x, y, z, w);
			}
			break;

		case OpSampler:
			dest = toFixed(inSamplers[i->src[1]]->get(toFloat(R[i->src[0]])));
			break;

		case OpSwizzle:
			{
				uint8_t ch = i->src[0];

				uint8_t cx = (ch & 0xc0) >> 6;
				uint8_t cy = (ch & 0x30) >> 4;
				uint8_t cz = (ch & 0x0c) >> 2;
				uint8_t cw = (ch & 0x03);

				const Fix4& s = R[i->src[1]];

				dest.set(
					s.e[cx],
					s.e[cy],
					s.e[cz],
					s.e[cw]
				);
			}
			break;

		case OpSet:
			{
				uint8_t ones = i->src[0];
				if (ones & 1)
					dest.x = c_one;
				if (ones & 2)
					dest.y = c_one;
				if (ones & 4)
					dest.z = c_one;
				if (ones & 8)
					dest.w = c_one;

				uint8_t zeros = i->src[1];
				if (zeros & 1)
					dest.x = c_zero;
				if (zeros & 2)
					dest.y = c_zero;
				if (zeros & 4)
					dest.z = c_zero;
				if (zeros & 8)
					dest.w = c_zero;
			}
			break;

		case OpExpandWithZero:
			{
				dest = R[i->src[0]];
				uint8_t ch = i->src[1];
				if (ch & 1)
					dest.x = c_zero;
				if (ch & 2)
					dest.y = c_zero;
				if (ch & 4)
					dest.z = c_zero;
				if (ch & 8)
					dest.w = c_zero;
			}
			break;

		case OpCompareGreaterEqual:
			{
				fp32_t inp = R[i->src[0]].x;
				fp32_t ref = R[i->src[1]].x;

				if (inp >= ref)
					dest.set(c_one, c_one, c_one, c_one);
				else
					dest.set(c_zero, c_zero, c_zero, c_zero);
			}
			break;

		case OpCompareGreater:
			{
				fp32_t inp = R[i->src[0]].x;
				fp32_t ref = R[i->src[1]].x;

				if (inp > ref)
					dest.set(c_one, c_one, c_one, c_one);
				else
					dest.set(c_zero, c_zero, c_zero, c_zero);
			}
			break;

		case OpCompareEqual:
			{
				fp32_t inp = R[i->src[0]].x;
				fp32_t ref = R[i->src[1]].x;

				if (inp == ref)
					dest.set(c_one, c_one, c_one, c_one);
				else
					dest.set(c_zero, c_zero, c_zero, c_zero);
			}
			break;

		case OpCompareNotEqual:
			{
				fp32_t inp = R[i->src[0]].x;
				fp32_t ref = R[i->src[1]].x;

				if (inp != ref)
					dest.set(c_one, c_one, c_one, c_one);
				else
					dest.set(c_zero, c_zero, c_zero, c_zero);
			}
			break;

		case OpJumpIfZero:
			{
				fp32_t v = R[i->dest].x;
				if (v == c_zero)
					std::advance(i, i->offset - 1);
			}
			break;

		case OpJump:
			std::advance(i, i->offset - 1);
			break;

		case OpDiscard:
			return false;

		case OpTrace:
			log::info << L"TRACE " << int32_t(i->dest) << Endl;
			break;

		default:
			T_BREAKPOINT;
		}
	}
	return true;
}

	}
}
