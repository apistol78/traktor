/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/TextFormat.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.TextFormat", TextFormat, ActionObjectRelay)

TextFormat::TextFormat(float letterSpacing, SwfTextAlignType align, float size)
:	ActionObjectRelay("TextFormat")
,	m_letterSpacing(letterSpacing)
,	m_align(align)
,	m_size(size)
{
}

	}
}
