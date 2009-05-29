#include <cmath>
#include <limits>
#include "Render/Sw/Core/Interpreter.h"
#include "Render/Sw/Core/IntrProgram.h"
#include "Render/Sw/Core/Sampler.h"
#include "Core/Math/Const.h"
#include "Core/Math/Matrix44.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct InternalImage
{
	IntrProgram program;
};

inline float clamp(float v, float mn, float mx)
{
	return min(max(v, mn), mx);
}

inline float frac(float v)
{
	if (v >= 0.0f)
		return v - floorf(v);
	else
		return -(-v - floorf(-v));
}

inline double fpow(double a, double b)
{
	int tmp = *(1 + (int*)&a);
	int tmp2 = (int)(b * (tmp - 1072632447) + 1072632447);
	double p = 0.0;
	*(1 + (int*)&p) = tmp2;
	return p;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Interpreter", Interpreter, Processor)

Processor::image_t Interpreter::compile(const IntrProgram& program) const
{
	InternalImage* img = new InternalImage();
	img->program = program;
	return img;
}

void Interpreter::destroy(image_t image) const
{
	InternalImage* img = reinterpret_cast< InternalImage* >(image);
	delete img;
}

void Interpreter::execute(
	const image_t image,
	const Vector4* inUniforms,
	const Vector4* inVaryings,
	const Ref< AbstractSampler >* inSamplers,
	Vector4* outVaryings
) const
{
	InternalImage* img = reinterpret_cast< InternalImage* >(image);
	T_ASSERT (img);

	Vector4 registers[256];
	#define R(i) registers[(i)]

	const std::vector< Instruction >& instructions = img->program.getInstructions();
	for (std::vector< Instruction >::const_iterator i = instructions.begin(); i != instructions.end(); ++i)
	{
		Vector4& dest = R(i->dest);
		switch (i->op)
		{
		case OpNop:
			break;

		case OpFetchConstant:
			dest = img->program.getConstant(i->src[0]);
			break;

		case OpFetchUniform:
			{
				uint32_t base = i->src[0];
				uint32_t size = i->src[1];
				T_ASSERT (base + size < 256);

				for (uint32_t j = 0; j < size; ++j)
					R(i->dest + j) = inUniforms[base + j];
			}
			break;

		case OpFetchIndexedUniform:
			{
				uint32_t base = i->src[0];
				uint32_t size = i->src[1];
				uint32_t index = uint32_t(R(i->src[2]).x() + Scalar(0.1f));
				T_ASSERT (base + index * size + size < 256);
				
				for (uint32_t j = 0; j < size; ++j)
					R(i->dest + j) = inUniforms[base + index * size + j];
			}
			break;

		case OpFetchVarying:
			T_ASSERT (i->src[0] < 4 + 2 + 8);
			dest = inVaryings[i->src[0]];
			break;

		case OpStoreVarying:
			T_ASSERT (i->dest < 4 + 2 + 8);
			outVaryings[i->dest] = R(i->src[0]);
			break;

		case OpMove:
			dest = R(i->src[0]);
			break;

		case OpAbs:
			dest = R(i->src[0]).absolute();
			break;

		case OpIncrement:
			dest += Scalar(1.0f);
			break;

		case OpDecrement:
			dest -= Scalar(1.0f);
			break;

		case OpAdd:
			dest = R(i->src[0]) + R(i->src[1]);
			break;

		case OpDiv:
			dest = R(i->src[0]) / R(i->src[1]);
			break;

		case OpMul:
			dest = R(i->src[0]) * R(i->src[1]);
			break;

		case OpMulAdd:
			dest = R(i->src[0]) * R(i->src[1]) + R(i->src[2]);
			break;

		case OpLog:
			{
				float v = std::log(R(i->src[0]).x());
				dest = Vector4(v, v, v, v);
			}
			break;

		//case OpLog2:
		//	dest.x =
		//	dest.y =
		//	dest.z =
		//	dest.w = 0.0f;
		//	break;

		case OpLog10:
			{
				float v = std::log10(R(i->src[0]).x());
				dest = Vector4(v, v, v, v);
			}
			break;

		case OpExp:
			dest.set(
				std::exp(R(i->src[0]).x()),
				std::exp(R(i->src[0]).y()),
				std::exp(R(i->src[0]).z()),
				std::exp(R(i->src[0]).w())
			);
			break;

		case OpFraction:
			dest.set(
				frac(R(i->src[0]).x()),
				frac(R(i->src[0]).y()),
				frac(R(i->src[0]).z()),
				frac(R(i->src[0]).w())
			);
			break;

		case OpNeg:
			dest = -R(i->src[0]);
			break;

		case OpPow:
			dest.set(
				fpow(R(i->src[0]).x(), R(i->src[1]).x()),
				fpow(R(i->src[0]).y(), R(i->src[1]).y()),
				fpow(R(i->src[0]).z(), R(i->src[1]).z()),
				fpow(R(i->src[0]).w(), R(i->src[1]).w())
			);
			break;

		case OpSqrt:
			dest.set(
				sqrtf(R(i->src[0]).x()),
				sqrtf(R(i->src[0]).y()),
				sqrtf(R(i->src[0]).z()),
				sqrtf(R(i->src[0]).w())
			);
			break;

		case OpSub:
			dest = R(i->src[0]) - R(i->src[1]);
			break;

		case OpAcos:
			dest.set(
				acosf(clamp(R(i->src[0]).x(), -1.0f, 1.0f)),
				acosf(clamp(R(i->src[0]).y(), -1.0f, 1.0f)),
				acosf(clamp(R(i->src[0]).z(), -1.0f, 1.0f)),
				acosf(clamp(R(i->src[0]).w(), -1.0f, 1.0f))
			);
			break;

		case OpAtan:
			dest.set(
				std::atan(R(i->src[0]).x()),
				std::atan(R(i->src[0]).y()),
				std::atan(R(i->src[0]).z()),
				std::atan(R(i->src[0]).w())
			);
			break;

		case OpCos:
			dest.set(
				cosf(R(i->src[0]).x()),
				cosf(R(i->src[0]).y()),
				cosf(R(i->src[0]).z()),
				cosf(R(i->src[0]).w())
			);
			break;

		case OpSin:
			dest.set(
				sinf(R(i->src[0]).x()),
				sinf(R(i->src[0]).y()),
				sinf(R(i->src[0]).z()),
				sinf(R(i->src[0]).w())
			);
			break;

		case OpTan:
			dest.set(
				std::tan(R(i->src[0]).x()),
				std::tan(R(i->src[0]).y()),
				std::tan(R(i->src[0]).z()),
				std::tan(R(i->src[0]).w())
			);
			break;

		case OpCross:
			dest = cross(R(i->src[0]), R(i->src[1]));
			break;

		case OpDot3:
			{
				Scalar a = dot3(R(i->src[0]), R(i->src[1]));
				dest.set(a, a, a, a);
			}
			break;

		case OpDot4:
			{
				Scalar a = dot4(R(i->src[0]), R(i->src[1]));
				dest.set(a, a, a, a);
			}
			break;

		case OpLength:
			{
				Scalar l = R(i->src[0]).length();
				dest.set(l, l, l, l);
			}
			break;

		case OpNormalize:
			{
				Scalar l = R(i->src[0]).length();
				if (abs(l) > FUZZY_EPSILON)
					dest = R(i->src[0]) / l;
				else
					dest.set(0.0f, 0.0f, 0.0f, 0.0f);
			}
			break;

		case OpTransform:
			{
				Matrix44 m(
					R(i->src[1] + 0),
					R(i->src[1] + 1),
					R(i->src[1] + 2),
					R(i->src[1] + 3)
				);
				dest = m * R(i->src[0]);
			}
			break;

		case OpTranspose:
			{
				Matrix44 m(
					R(i->src[0] + 0),
					R(i->src[0] + 1),
					R(i->src[0] + 2),
					R(i->src[0] + 3)
				);

				Matrix44 t = m.transpose();

				R(i->dest + 0) = t.axisX();
				R(i->dest + 1) = t.axisY();
				R(i->dest + 2) = t.axisZ();
				R(i->dest + 3) = t.translation();
			}
			break;

		case OpClamp:
			{
				Vector4 mn = img->program.getConstant(i->src[1]);
				Vector4 mx = img->program.getConstant(i->src[2]);
				dest.set(
					clamp(R(i->src[0]).x(), mn.x(), mx.x()),
					clamp(R(i->src[0]).y(), mn.y(), mx.y()),
					clamp(R(i->src[0]).z(), mn.z(), mx.z()),
					clamp(R(i->src[0]).w(), mn.w(), mx.w())
				);
			}
			break;

		case OpLerp:
			{
				Scalar s = R(i->src[0]).x();
				dest = R(i->src[1]) * (Scalar(1.0f) - s) + R(i->src[2]) * s;
			}
			break;

		case OpMixIn:
			{
				float x = R(i->src[0]).x();
				float y = R(i->src[1]).x();
				float z = R(i->src[2]).x();
				float w = R(i->src[3]).x();
				dest.set(x, y, z, w);
			}
			break;

		case OpMax:
			dest = max(R(i->src[0]), R(i->src[1]));
			break;

		case OpSampler:
			{
				if (inSamplers)
				{
					const AbstractSampler* sampler = inSamplers[i->src[1]];
					if (sampler)
					{
						dest = sampler->get(R(i->src[0]));
						break;
					}
				}
				dest = Vector4::zero();
			}
			break;

		case OpSwizzle:
			{
				uint8_t ch = i->src[0];

				uint8_t cx = (ch & 0xc0) >> 6;
				uint8_t cy = (ch & 0x30) >> 4;
				uint8_t cz = (ch & 0x0c) >> 2;
				uint8_t cw = (ch & 0x03);

				const Vector4& s = R(i->src[1]);

				dest.set(
					s[cx],
					s[cy],
					s[cz],
					s[cw]
				);
			}
			break;

		case OpSet:
			{
				dest = Vector4(0.0f, 0.0f, 0.0f, 0.0f);

				uint8_t ones = i->src[0];
				if (ones & 1)
					dest += Vector4(1.0f, 0.0f, 0.0f, 0.0f);
				if (ones & 2)
					dest += Vector4(0.0f, 1.0f, 0.0f, 0.0f);
				if (ones & 4)
					dest += Vector4(0.0f, 0.0f, 1.0f, 0.0f);
				if (ones & 8)
					dest += Vector4(0.0f, 0.0f, 0.0f, 1.0f);

				uint8_t zeros = i->src[1];
				if (zeros & 1)
					dest *= Vector4(0.0f, 1.0f, 1.0f, 1.0f);
				if (zeros & 2)
					dest *= Vector4(1.0f, 0.0f, 1.0f, 1.0f);
				if (zeros & 4)
					dest *= Vector4(1.0f, 1.0f, 0.0f, 1.0f);
				if (zeros & 8)
					dest *= Vector4(1.0f, 1.0f, 1.0f, 0.0f);
			}
			break;

		case OpExpandWithZero:
			{
				dest = R(i->src[0]);
				uint8_t ch = i->src[1];
				if (ch & 1)
					dest *= Vector4(0.0f, 1.0f, 1.0f, 1.0f);
				if (ch & 2)
					dest *= Vector4(1.0f, 0.0f, 1.0f, 1.0f);
				if (ch & 4)
					dest *= Vector4(1.0f, 1.0f, 0.0f, 1.0f);
				if (ch & 8)
					dest *= Vector4(1.0f, 1.0f, 1.0f, 0.0f);
			}
			break;

		case OpSplat:
			{
				float sp = R(i->src[0])[i->src[1]];
				dest.set(sp, sp, sp, sp);
			}
			break;

		case OpCompareGreaterEqual:
			{
				Scalar inp = R(i->src[0]).x();
				Scalar ref = R(i->src[1]).x();

				if (inp >= ref)
					dest.set(1.0f, 1.0f, 1.0f, 1.0f);
				else
					dest.set(0.0f, 0.0f, 0.0f, 0.0f);
			}
			break;

		case OpCompareGreater:
			{
				Scalar inp = R(i->src[0]).x();
				Scalar ref = R(i->src[1]).x();

				if (inp > ref)
					dest.set(1.0f, 1.0f, 1.0f, 1.0f);
				else
					dest.set(0.0f, 0.0f, 0.0f, 0.0f);
			}
			break;

		case OpCompareEqual:
			{
				Scalar inp = R(i->src[0]).x();
				Scalar ref = R(i->src[1]).x();

				if (abs(inp - ref) < FUZZY_EPSILON)
					dest.set(1.0f, 1.0f, 1.0f, 1.0f);
				else
					dest.set(0.0f, 0.0f, 0.0f, 0.0f);
			}
			break;

		case OpCompareNotEqual:
			{
				Scalar inp = R(i->src[0]).x();
				Scalar ref = R(i->src[1]).x();

				if (abs(inp - ref) > FUZZY_EPSILON)
					dest.set(1.0f, 1.0f, 1.0f, 1.0f);
				else
					dest.set(0.0f, 0.0f, 0.0f, 0.0f);
			}
			break;

		case OpJumpIfZero:
			{
				if (abs(dest.x()) <= FUZZY_EPSILON)
					std::advance(i, i->offset - 1);
			}
			break;

		case OpJump:
			std::advance(i, i->offset - 1);
			break;

		case OpTrace:
			log::info << L"TRACE " << int32_t(i->dest) << Endl;
			break;

		default:
			T_ASSERT (0);
		}
	}
}

	}
}
