#include <cmath>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Math/Matrix44.h"
#include "Render/Sw/Core/Interpreter.h"
#include "Render/Sw/Core/IntrProgram.h"
#include "Render/Sw/Core/Sampler.h"

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

float T_FORCE_INLINE clamp(float v, float mn, float mx)
{
	return min(max(v, mn), mx);
}

float T_FORCE_INLINE frac(float v)
{
	if (v >= 0.0f)
		return v - floorf(v);
	else
		return -(-v - floorf(-v));
}

float T_FORCE_INLINE fpow(float a, float b)
{
	return std::powf(a, b);
}

void T_FORCE_INLINE checkRegister(const Vector4& r)
{
	T_ASSERT (!(isNan(r.x()) || isInfinite(r.x())));
	T_ASSERT (!(isNan(r.y()) || isInfinite(r.y())));
	T_ASSERT (!(isNan(r.z()) || isInfinite(r.z())));
	T_ASSERT (!(isNan(r.w()) || isInfinite(r.w())));
}

#define CHECK(r) checkRegister(r)

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

bool Interpreter::execute(
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

	Vector4 registers[256];
	#define R(i) registers[(i)]

	const AlignedVector< Instruction >& instructions = img->program.getInstructions();
	uint32_t executed = 0;

	for (
		AlignedVector< Instruction >::const_iterator i = instructions.begin();
		i != instructions.end() && executed < 500;
		++i, ++executed
	)
	{
		Vector4& dest = R(i->dest);
		switch (i->op)
		{
		case OpNop:
			break;

		case OpFetchConstant:
			dest = img->program.getConstant(i->src[0]);
			CHECK(dest);
			break;

		case OpFetchInstance:
			dest = Vector4(
				float(instance),
				float(instance),
				float(instance),
				float(instance)
			);
			CHECK(dest);
			break;

		case OpFetchTargetSize:
			dest = targetSize;
			CHECK(dest);
			break;

		case OpFetchFragmentPosition:
			dest = fragmentPosition;
			CHECK(dest);
			break;

		case OpFetchTextureSize:
			{
				if (inSamplers)
				{
					const AbstractSampler* sampler = inSamplers[i->src[0]];
					if (sampler)
					{
						dest = sampler->getSize();
						CHECK(dest);
						break;
					}
				}
				dest = Vector4::zero();
				CHECK(dest);
			}
			break;

		case OpFetchUniform:
			{
				uint32_t base = i->src[0];
				uint32_t size = i->src[1];
				T_ASSERT (base + size < 256);

				for (uint32_t j = 0; j < size; ++j)
				{
					R(i->dest + j) = inUniforms[base + j];
					CHECK(R(i->dest + j));
				}
			}
			break;

		case OpFetchIndexedUniform:
			{
				uint32_t base = i->src[0];
				uint32_t size = i->src[1];
				uint32_t index = uint32_t(R(i->src[2]).x() + Scalar(0.1f));
				T_ASSERT (base + index * size + size < 256);
				
				for (uint32_t j = 0; j < size; ++j)
				{
					R(i->dest + j) = inUniforms[base + index * size + j];
					CHECK(R(i->dest + j));
				}
			}
			break;

		case OpFetchVarying:
			T_ASSERT (i->src[0] < 4 + 2 + 8);
			dest = inVaryings[i->src[0]];
			CHECK(dest);
			break;

		case OpStoreVarying:
			T_ASSERT (i->dest < 4 + 2 + 8);
			outVaryings[i->dest] = R(i->src[0]);
			CHECK(outVaryings[i->dest]);
			break;

		case OpMove:
			dest = R(i->src[0]);
			CHECK(dest);
			break;

		case OpAbs:
			dest = R(i->src[0]).absolute();
			CHECK(dest);
			break;

		case OpIncrement:
			dest += Scalar(1.0f);
			CHECK(dest);
			break;

		case OpDecrement:
			dest -= Scalar(1.0f);
			CHECK(dest);
			break;

		case OpAdd:
			dest = R(i->src[0]) + R(i->src[1]);
			CHECK(dest);
			break;

		case OpDiv:
			{
				Vector4 denom = R(i->src[1]);
				for (int j = 0; j < 4; ++j)
				{
					if (abs(denom[j]) <= Scalar(FUZZY_EPSILON))
						denom.set(j, Scalar(1.0f));
				}
				dest = R(i->src[0]) / denom;
				CHECK(dest);
			}
			break;

		case OpMul:
			dest = R(i->src[0]) * R(i->src[1]);
			CHECK(dest);
			break;

		case OpMulAdd:
			dest = R(i->src[0]) * R(i->src[1]) + R(i->src[2]);
			CHECK(dest);
			break;

		case OpLog:
			{
				float v = std::log(R(i->src[0]).x());
				dest = Vector4(v, v, v, v);
				CHECK(dest);
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
				CHECK(dest);
			}
			break;

		case OpExp:
			dest.set(
				std::exp(R(i->src[0]).x()),
				std::exp(R(i->src[0]).y()),
				std::exp(R(i->src[0]).z()),
				std::exp(R(i->src[0]).w())
			);
			CHECK(dest);
			break;

		case OpFraction:
			dest.set(
				frac(R(i->src[0]).x()),
				frac(R(i->src[0]).y()),
				frac(R(i->src[0]).z()),
				frac(R(i->src[0]).w())
			);
			CHECK(dest);
			break;

		case OpNeg:
			dest = -R(i->src[0]);
			CHECK(dest);
			break;

		case OpPow:
			dest.set(
				fpow(R(i->src[0]).x(), R(i->src[1]).x()),
				fpow(R(i->src[0]).y(), R(i->src[1]).y()),
				fpow(R(i->src[0]).z(), R(i->src[1]).z()),
				fpow(R(i->src[0]).w(), R(i->src[1]).w())
			);
			CHECK(dest);
			break;

		case OpSqrt:
			{
				float x = std::max< float >(R(i->src[0]).x(), 0.0f);
				float y = std::max< float >(R(i->src[0]).y(), 0.0f);
				float z = std::max< float >(R(i->src[0]).z(), 0.0f);
				float w = std::max< float >(R(i->src[0]).w(), 0.0f);
				dest.set(
					sqrtf(x),
					sqrtf(y),
					sqrtf(z),
					sqrtf(w)
				);
				CHECK(dest);
			}
			break;

		case OpRecipSqrt:
			{
				Scalar x = max(R(i->src[0]).x(), Scalar(0.0f));
				Scalar y = max(R(i->src[0]).y(), Scalar(0.0f));
				Scalar z = max(R(i->src[0]).z(), Scalar(0.0f));
				Scalar w = max(R(i->src[0]).w(), Scalar(0.0f));
				dest.set(
					reciprocalSquareRoot(x),
					reciprocalSquareRoot(y),
					reciprocalSquareRoot(z),
					reciprocalSquareRoot(w)
				);
				CHECK(dest);
			}
			break;

		case OpSub:
			dest = R(i->src[0]) - R(i->src[1]);
			CHECK(dest);
			break;

		case OpAcos:
			dest.set(
				acosf(clamp(R(i->src[0]).x(), -1.0f, 1.0f)),
				acosf(clamp(R(i->src[0]).y(), -1.0f, 1.0f)),
				acosf(clamp(R(i->src[0]).z(), -1.0f, 1.0f)),
				acosf(clamp(R(i->src[0]).w(), -1.0f, 1.0f))
			);
			CHECK(dest);
			break;

		case OpAtan:
			dest.set(
				std::atan(R(i->src[0]).x()),
				std::atan(R(i->src[0]).y()),
				std::atan(R(i->src[0]).z()),
				std::atan(R(i->src[0]).w())
			);
			CHECK(dest);
			break;

		case OpCos:
			dest.set(
				cosf(R(i->src[0]).x()),
				cosf(R(i->src[0]).y()),
				cosf(R(i->src[0]).z()),
				cosf(R(i->src[0]).w())
			);
			CHECK(dest);
			break;

		case OpSin:
			dest.set(
				sinf(R(i->src[0]).x()),
				sinf(R(i->src[0]).y()),
				sinf(R(i->src[0]).z()),
				sinf(R(i->src[0]).w())
			);
			CHECK(dest);
			break;

		case OpTan:
			dest.set(
				std::tan(R(i->src[0]).x()),
				std::tan(R(i->src[0]).y()),
				std::tan(R(i->src[0]).z()),
				std::tan(R(i->src[0]).w())
			);
			CHECK(dest);
			break;

		case OpCross:
			dest = cross(R(i->src[0]), R(i->src[1]));
			CHECK(dest);
			break;

		case OpDot3:
			{
				Scalar a = dot3(R(i->src[0]), R(i->src[1]));
				dest.set(a, a, a, a);
				CHECK(dest);
			}
			break;

		case OpDot4:
			{
				Scalar a = dot4(R(i->src[0]), R(i->src[1]));
				dest.set(a, a, a, a);
				CHECK(dest);
			}
			break;

		case OpLength:
			{
				Scalar l = R(i->src[0]).length();
				dest.set(l, l, l, l);
				CHECK(dest);
			}
			break;

		case OpNormalize:
			{
				Scalar l = R(i->src[0]).length();
				if (abs(l) > FUZZY_EPSILON)
					dest = R(i->src[0]) / l;
				else
					dest.set(0.0f, 0.0f, 0.0f, 0.0f);
				CHECK(dest);
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
				CHECK(dest);
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
				CHECK(dest);
			}
			break;

		case OpTrunc:
			{
				float x = std::floorf(R(i->src[0]).x());
				float y = std::floorf(R(i->src[0]).y());
				float z = std::floorf(R(i->src[0]).z());
				float w = std::floorf(R(i->src[0]).w());
				dest.set(x, y, z, w);
				CHECK(dest);
			}
			break;

		case OpRound:
			{
				float x = std::floorf(R(i->src[0]).x() + 0.5f);
				float y = std::floorf(R(i->src[0]).y() + 0.5f);
				float z = std::floorf(R(i->src[0]).z() + 0.5f);
				float w = std::floorf(R(i->src[0]).w() + 0.5f);
				dest.set(x, y, z, w);
				CHECK(dest);
			}
			break;

		case OpLerp:
			{
				Scalar s = R(i->src[0]).x();
				dest = R(i->src[1]) * (Scalar(1.0f) - s) + R(i->src[2]) * s;
				CHECK(dest);
			}
			break;

		case OpMixIn:
			{
				float x = R(i->src[0]).x();
				float y = R(i->src[1]).x();
				float z = R(i->src[2]).x();
				float w = R(i->src[3]).x();
				dest.set(x, y, z, w);
				CHECK(dest);
			}
			break;

		case OpMin:
			dest = min(R(i->src[0]), R(i->src[1]));
			CHECK(dest);
			break;

		case OpMax:
			dest = max(R(i->src[0]), R(i->src[1]));
			CHECK(dest);
			break;

		case OpStep:
			{
				float x = R(i->src[0]).x() >= R(i->src[1]).x() ? 1.0f : 0.0f;
				float y = R(i->src[0]).y() >= R(i->src[1]).y() ? 1.0f : 0.0f;
				float z = R(i->src[0]).z() >= R(i->src[1]).z() ? 1.0f : 0.0f;
				float w = R(i->src[0]).w() >= R(i->src[1]).w() ? 1.0f : 0.0f;
				dest.set(x, y, z, w);
				CHECK(dest);
			}
			break;

		case OpSign:
			{
				float x = sign(R(i->src[0]).x());
				float y = sign(R(i->src[1]).y());
				float z = sign(R(i->src[2]).z());
				float w = sign(R(i->src[3]).z());
				dest.set(x, y, z, w);
				CHECK(dest);
			}
			break;

		case OpSampler:
			{
				if (inSamplers)
				{
					const AbstractSampler* sampler = inSamplers[i->src[1]];
					if (sampler)
					{
						dest = sampler->get(R(i->src[0]));
						CHECK(dest);
						break;
					}
				}
				dest = Vector4::zero();
				CHECK(dest);
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
				CHECK(dest);
			}
			break;

		case OpSet:
			{
				uint8_t ones = i->src[0];
				if (ones & 1)
					dest.set(0, Scalar(1.0f));
				if (ones & 2)
					dest.set(1, Scalar(1.0f));
				if (ones & 4)
					dest.set(2, Scalar(1.0f));
				if (ones & 8)
					dest.set(3, Scalar(1.0f));

				uint8_t zeros = i->src[1];
				if (zeros & 1)
					dest.set(0, Scalar(0.0f));
				if (zeros & 2)
					dest.set(1, Scalar(0.0f));
				if (zeros & 4)
					dest.set(2, Scalar(0.0f));
				if (zeros & 8)
					dest.set(3, Scalar(0.0f));

				CHECK(dest);
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
				CHECK(dest);
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

				CHECK(dest);
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

				CHECK(dest);
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

				CHECK(dest);
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

				CHECK(dest);
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

		case OpDiscard:
			return false;

		case OpTrace:
			log::info << L"TRACE " << int32_t(i->dest) << Endl;
			break;

		default:
			T_ASSERT (0);
		}
	}
	return true;
}

	}
}
