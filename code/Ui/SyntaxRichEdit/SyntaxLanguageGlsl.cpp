/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/SyntaxRichEdit/SyntaxLanguageGlsl.h"

namespace traktor::ui
{
	namespace
	{

const wchar_t* c_glslIntrinsics[] =
{
	L"determinant",
	L"texCUBEbias",
	L"texCUBEgrad",
	L"texCUBEproj",
	L"ddx_coarse",
	L"ddy_coarse",
	L"smoothstep",
	L"texCUBElod",
	L"textureLod",
	L"countbits",
	L"normalize",
	L"tex1Dbias",
	L"tex1Dgrad",
	L"tex1Dproj",
	L"tex2Dbias",
	L"tex2Dgrad",
	L"tex2Dproj",
	L"tex3Dbias",
	L"tex3Dproj",
	L"transpose",
	L"asdouble",
	L"ddx_fine",
	L"ddy_fine",
	L"distance",
	L"isfinite",
	L"saturate",
	L"tex1Dlod",
	L"tex2Dlod",
	L"tex3Dlod",
	L"asfloat",
	L"degrees",
	L"discard",
	L"radians",
	L"reflect",
	L"refract",
	L"texCUBE",
	L"Texture",
	L"asuint",
	L"errorf",
	L"fwidth",
	L"length",
	L"printf",
	L"Sample",
	L"sincos",
	L"abort",
	L"asint",
	L"atan2",
	L"clamp",
	L"cross",
	L"floor",
	L"fract",
	L"frexp",
	L"isinf",
	L"isnan",
	L"ldexp",
	L"log10",
	L"noise",
	L"round",
	L"rsqrt",
	L"tex1D",
	L"tex2D",
	L"tex3D",
	L"trunc",
	L"acos",
	L"asin",
	L"atan",
	L"ceil",
	L"clip",
	L"cosh",
	L"exp2",
	L"fmod",
	L"frac",
	L"lerp",
	L"log2",
	L"modf",
	L"sinh",
	L"sqrt",
	L"step",
	L"tanh",
	L"abs",
	L"all",
	L"any",
	L"cos",
	L"ddx",
	L"ddy",
	L"dot",
	L"dst",
	L"exp",
	L"fma",
	L"lit",
	L"log",
	L"mad",
	L"max",
	L"min",
	L"mix",
	L"mod",
	L"mul",
	L"pow",
	L"rcp",
	L"sin",
	L"tan",
	nullptr
};

const wchar_t* c_glslKeywords[] =
{
	L"noperspective",
	L"attribute",
	L"invariant",
	L"precision",
	L"centroid",
	L"continue",
	L"default",
	L"readonly",
	L"uniform",
	L"varying",
	L"discard",
	L"mediump",
	L"reflect",
	L"refract",
	L"texCUBE",
	L"Texture",
	L"layout",
	L"smooth",
	L"switch",
	L"return",
	L"struct",
	L"asuint",
	L"errorf",
	L"fwidth",
	L"length",
	L"printf",
	L"Sample",
	L"sincos",
	L"break",
	L"while",
	L"inout",
	L"false",
	L"highp",
	L"abort",
	L"asint",
	L"atan2",
	L"clamp",
	L"cross",
	L"floor",
	L"fract",
	L"frexp",
	L"isinf",
	L"isnan",
	L"ldexp",
	L"log10",
	L"noise",
	L"round",
	L"rsqrt",
	L"tex1D",
	L"tex2D",
	L"tex3D",
	L"trunc",
	L"flat",
	L"case",
	L"else",
	L"true",
	L"lowp",
	L"acos",
	L"asin",
	L"atan",
	L"ceil",
	L"clip",
	L"cosh",
	L"exp2",
	L"fmod",
	L"frac",
	L"lerp",
	L"log2",
	L"modf",
	L"sinh",
	L"sqrt",
	L"step",
	L"tanh",
	L"for",
	L"out",
	L"abs",
	L"all",
	L"any",
	L"cos",
	L"ddx",
	L"ddy",
	L"dot",
	L"dst",
	L"exp",
	L"fma",
	L"lit",
	L"log",
	L"mad",
	L"max",
	L"min",
	L"mix",
	L"mul",
	L"pow",
	L"rcp",
	L"sin",
	L"tan",
	L"do",
	L"if",
	L"in",
	nullptr
};

const wchar_t* c_glslTypes[] =
{
	L"sampler1DArrayShadow",
	L"sampler2DArrayShadow",
	L"sampler2DRectShadow",
	L"samplerCubeShadow",
	L"sampler1DShadow",
	L"sampler2DShadow",
	L"isampler1DArray",
	L"isampler2DArray",
	L"usampler1DArray",
	L"usampler2DArray",
	L"sampler1DArray",
	L"sampler2DArray",
	L"isampler2DRect",
	L"usampler2DRect",
	L"isamplerBuffer",
	L"usamplerBuffer",
	L"sampler2DRect",
	L"samplerBuffer",
	L"isamplerCube",
	L"usamplerCube",
	L"samplerCube",
	L"textureCube",
	L"isampler1D",
	L"isampler2D",
	L"isampler3D",
	L"usampler1D",
	L"usampler2D",
	L"usampler3D",
	L"sampler1D",
	L"sampler2D",
	L"sampler3D",
	L"texture2D",
	L"texture3D",
	L"sampler",
	L"mat2x2",
	L"mat2x3",
	L"mat2x4",
	L"mat3x2",
	L"mat3x3",
	L"mat3x4",
	L"mat4x2",
	L"mat4x3",
	L"mat4x4",
	L"buffer",
	L"const",
	L"float",
	L"ivec2",
	L"ivec3",
	L"ivec4",
	L"bvec2",
	L"bvec3",
	L"bvec4",
	L"uvec2",
	L"uvec3",
	L"uvec4",
	L"ceil",
	L"clip",
	L"cosh",
	L"exp2",
	L"fmod",
	L"frac",
	L"lerp",
	L"log2",
	L"modf",
	L"sinh",
	L"sqrt",
	L"step",
	L"tanh",
	L"void",
	L"bool",
	L"mat2",
	L"mat3",
	L"mat4",
	L"vec2",
	L"vec3",
	L"vec4",
	L"uint",
	L"for",
	L"out",
	L"abs",
	L"all",
	L"any",
	L"cos",
	L"ddx",
	L"ddy",
	L"dot",
	L"dst",
	L"exp",
	L"fma",
	L"lit",
	L"log",
	L"mad",
	L"max",
	L"min",
	L"mix",
	L"mul",
	L"pow",
	L"rcp",
	L"sin",
	L"tan",
	L"int",
	L"do",
	L"if",
	L"in",
	nullptr
};

class SyntaxLanguageContextGlsl: public RefCountImpl< SyntaxLanguage::IContext >
{
public:
	bool m_blockComment = false;
};

bool match(const std::wstring_view& text, const std::wstring_view& patt)
{
	if (text.length() < patt.length())
		return false;

	for (size_t i = 0; i < patt.length(); ++i)
	{
		if (text[i] != patt[i])
			return false;
	}

	return true;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ui.SyntaxLanguageGlsl", 0, SyntaxLanguageGlsl, SyntaxLanguage)

std::wstring SyntaxLanguageGlsl::lineComment() const
{
	return L"//";
}

Ref< SyntaxLanguage::IContext > SyntaxLanguageGlsl::createContext() const
{
	return new SyntaxLanguageContextGlsl();
}

bool SyntaxLanguageGlsl::consume(SyntaxLanguage::IContext* context, const std::wstring& text, State& outState, int& outConsumedChars) const
{
	SyntaxLanguageContextGlsl* cx = static_cast< SyntaxLanguageContextGlsl* >(context);
	int32_t ln = (int32_t)text.length();
	T_ASSERT(ln > 0);

	if (cx->m_blockComment)
	{
		if (match(text, L"*/"))
		{
			cx->m_blockComment = false;

			outState = StBlockComment;
			outConsumedChars = 2;
			return true;
		}
		else
		{
			outState = StBlockComment;
			outConsumedChars = 1;
			return true;
		}
	}

	// Line comment.
	if (match(text, L"//"))
	{
		outState = StLineComment;
		outConsumedChars = 2;
		for (int32_t i = 2; i < ln; ++i)
		{
			if (text[i] == L'\n' || text[i] == L'\r')
				break;
			++outConsumedChars;
		}
		return true;
	}

	// Block comment.
	if (match(text, L"/*"))
	{
		cx->m_blockComment = true;

		outState = StBlockComment;
		outConsumedChars = 2;
		return true;
	}

	// String
	if (text[0] == L'\"')
	{
		outState = StString;
		outConsumedChars = 1;

		for (int32_t i = 1; i < ln; ++i)
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

		int32_t i = 1;

		// Integer or float.
		for (; i < ln && text[i] >= L'0' && text[i] <= L'9'; ++i)
			++outConsumedChars;

		// Fractional
		if (i < ln && text[i] == L'.')
		{
			++outConsumedChars;
			for (++i; i < ln && text[i] >= L'0' && text[i] <= L'9'; ++i)
				++outConsumedChars;
			if (i < ln && text[i] == L'f')
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
	if (ws == 0)
	{
		outState = StDefault;
		outConsumedChars = 1;
		return true;
	}
	std::wstring word = text.substr(0, ws);

	if (word[0] == L'#')
	{
		outState = StPreprocessor;
		outConsumedChars = int(ws);
		return true;		
	}

	for (const wchar_t** k = c_glslKeywords; *k != nullptr; ++k)
	{
		if (word == *k)
		{
			outState = StKeyword;
			outConsumedChars = int(ws);
			return true;
		}
	}

	for (const wchar_t** k = c_glslTypes; *k != nullptr; ++k)
	{
		if (word == *k)
		{
			outState = StType;
			outConsumedChars = int(ws);
			return true;
		}
	}

	for (const wchar_t** k = c_glslIntrinsics; *k != nullptr; ++k)
	{
		if (word == *k)
		{
			outState = StFunction;
			outConsumedChars = int(ws);
			return true;
		}
	}

	// Default as text.
	outState = StDefault;
	outConsumedChars = int(ws);
	return true;
}

void SyntaxLanguageGlsl::outline(int32_t line, const std::wstring& text, std::list< SyntaxOutline >& outOutline) const
{
}

}
