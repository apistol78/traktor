#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguageHlsl.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ui.custom.SyntaxLanguageHlsl", 0, SyntaxLanguageHlsl, SyntaxLanguage)

std::wstring SyntaxLanguageHlsl::lineComment() const
{
	return L"//";
}

bool SyntaxLanguageHlsl::consume(const std::wstring& text, State& outState, int& outConsumedChars) const
{
	int ln = int(text.length());
	T_ASSERT (ln > 0);

	// Line comment.
	if (ln >= 2)
	{
		if (text[0] == L'/' && text[1] == L'/')
		{
			outState = StComment;
			outConsumedChars = 2;
			for (int i = 2; i < ln; ++i)
			{
				if (text[i] == L'\n' || text[i] == L'\r')
					break;
				++outConsumedChars;
			}
			return true;
		}
	}

	// String
	if (text[0] == L'\"')
	{
		outState = StString;
		outConsumedChars = 1;

		for (int i = 1; i < ln; ++i)
		{
			++outConsumedChars;
			if (text[i] == L'\"')
				break;
		}

		return true;
	}

	// Number
	if (text[0] >= L'0' && text[0] <= L'9')
	{
		outState = StNumber;
		outConsumedChars = 1;

		int i = 1;

		// Integer or float.
		for (; (i < ln && (text[i] >= L'0' && text[i] <= L'9')) || text[i] == L'.'; ++i)
			++outConsumedChars;

		// Fractional
		if (text[i] == L'.')
		{
			for (; i < ln && text[i] >= L'0' && text[i] <= L'9'; ++i)
				++outConsumedChars;
		}

		return true;
	}

	// White space.
	if (text[0] == L' ' || text[0] == L'\t' || text[0] == L'\n' || text[0] == L'\r')
	{
		outState = StDefault;
		outConsumedChars = 1;
		return true;
	}

	// Special numbers or keywords.
	size_t i = text.find_first_of(L" \t\n\r();");
	size_t ws = (i != text.npos) ? i : ln;

	std::wstring word = text.substr(0, ws);
	if (
		word == L"break" ||
		word == L"case" ||
		word == L"const" ||
		word == L"continue" ||
		word == L"else" ||
		word == L"for" ||
		word == L"if" ||
		word == L"return" ||
		word == L"struct" ||
		word == L"switch" ||
		word == L"uniform" ||
		word == L"while"
	)
	{
		outState = StKeyword;
		outConsumedChars = int(ws);
		return true;
	}

	if (
		word == L"cbuffer" ||
		word == L"float" ||
		word == L"float2" ||
		word == L"float3" ||
		word == L"float4" ||
		word == L"float4x4" ||
		word == L"half" ||
		word == L"half2" ||
		word == L"half3" ||
		word == L"half4" ||
		word == L"half4x4" ||
		word == L"texture" ||
		word == L"sampler" ||
		word == L"void"
	)
	{
		outState = StType;
		outConsumedChars = int(ws);
		return true;
	}

	if (
		word == L"abort" ||
		word == L"abs" ||
		word == L"acos" ||
		word == L"all" ||
		word == L"any" ||
		word == L"asdouble" ||
		word == L"asfloat" ||
		word == L"asin" ||
		word == L"asint" ||
		word == L"asuint" ||
		word == L"atan" ||
		word == L"atan2" ||
		word == L"ceil" ||
		word == L"clamp" ||
		word == L"clip" ||
		word == L"cos" ||
		word == L"cosh" ||
		word == L"countbits" ||
		word == L"cross" ||
		word == L"ddx" ||
		word == L"ddx_coarse" ||
		word == L"ddx_fine" ||
		word == L"ddy" ||
		word == L"ddy_coarse" ||
		word == L"ddy_fine" ||
		word == L"degrees" ||
		word == L"determinant" ||
		word == L"discard" ||
		word == L"distance" ||
		word == L"dot" ||
		word == L"dst" ||
		word == L"errorf" ||
		word == L"exp" ||
		word == L"exp2" ||
		word == L"floor" ||
		word == L"fma" ||
		word == L"fmod" ||
		word == L"frac" ||
		word == L"frexp" ||
		word == L"fwidth" ||
		word == L"isfinite" ||
		word == L"isinf" ||
		word == L"isnan" ||
		word == L"ldexp" ||
		word == L"length" ||
		word == L"lerp" ||
		word == L"lit" ||
		word == L"log" ||
		word == L"log10" ||
		word == L"log2" ||
		word == L"mad" ||
		word == L"max" ||
		word == L"min" ||
		word == L"modf" ||
		word == L"mul" ||
		word == L"noise" ||
		word == L"normalize" ||
		word == L"pow" ||
		word == L"printf" ||
		word == L"radians" ||
		word == L"rcp" ||
		word == L"reflect" ||
		word == L"refract" ||
		word == L"round" ||
		word == L"rsqrt" ||
		word == L"saturate" ||
		word == L"sin" ||
		word == L"sincos" ||
		word == L"sinh" ||
		word == L"smoothstep" ||
		word == L"sqrt" ||
		word == L"step" ||
		word == L"tan" ||
		word == L"tanh" ||
		word == L"tex1D" ||
		word == L"tex1Dbias" ||
		word == L"tex1Dgrad" ||
		word == L"tex1Dlod" ||
		word == L"tex1Dproj" ||
		word == L"tex2D" ||
		word == L"tex2Dbias" ||
		word == L"tex2Dgrad" ||
		word == L"tex2Dlod" ||
		word == L"tex2Dproj" ||
		word == L"tex3D" ||
		word == L"tex3Dbias" ||
		word == L"tex3Dlod" ||
		word == L"tex3Dproj" ||
		word == L"texCUBE" ||
		word == L"texCUBEbias" ||
		word == L"texCUBEgrad" ||
		word == L"texCUBElod" ||
		word == L"texCUBEproj" ||
		word == L"transpose" ||
		word == L"trunc" ||
		word == L"Texture" ||
		word == L"Sample"
	)
	{
		outState = StFunction;
		outConsumedChars = int(ws);
		return true;
	}

	// Default as text.
	outState = StDefault;
	outConsumedChars = 1;
	return true;
}

void SyntaxLanguageHlsl::outline(int32_t line, const std::wstring& text, std::list< SyntaxOutline >& outOutline) const
{
}

		}
	}
}
