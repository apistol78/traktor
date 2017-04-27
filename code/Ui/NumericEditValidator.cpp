/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/NumericEditValidator.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.NumericEditValidator", NumericEditValidator, EditValidator)

NumericEditValidator::NumericEditValidator(bool floatPoint, float min, float max, int decimals)
:	m_floatPoint(floatPoint)
,	m_min(min)
,	m_max(max)
,	m_decimals(decimals)
{
}

EditValidator::Result NumericEditValidator::validate(const std::wstring& text) const
{
	EditValidator::Result result = EditValidator::VrOk;
	int state = 0;
	int pos = 0;
	int dec = 0;
	bool dot = false;

	while (state >= 0 && pos < int(text.length()))
	{
		wchar_t ch = text[pos];
		switch (state)
		{
		// Trim leading whitespace.
		case 0:
			if (ch != L' ' && ch != L'\t')
				state = 1;
			else
				pos++;
			break;

		// Check for sign.
		case 1:
			if (ch == L'-')
			{
				if (m_min < 0)
				{
					pos++;
					state = 2;
				}
				else
				{
					result = EditValidator::VrInvalid;
					state = -1;
				}
			}
			else
				state = 2;
			break;

		// Consume digits.
		case 2:
			if (ch >= L'0' && ch <= L'9')
			{
				if (dot == true)
				{
					if (dec++ >= m_decimals)
					{
						result = EditValidator::VrInvalid;
						state = -1;
					}
				}
				pos++;
			}
			else
				state = 3;
			break;
		
		// Consume single dot.
		case 3:
			if (ch == L'.')
			{
				if (m_floatPoint == true && dot == false)
				{
					dot = true;
					pos++;
					state = 2;
				}
				else
				{
					result = EditValidator::VrInvalid;
					state = -1;
				}
			}
			else
			{
				result = EditValidator::VrInvalid;
				state = -1;
			}
			break;
		}
	}

	// Check number within range.
	if (result == EditValidator::VrOk)
	{
		if (!text.empty())
		{
			float v = parseString< float >(text);
			if (v < m_min || v > m_max)
				result = EditValidator::VrInvalid;
		}
		else
			result = EditValidator::VrIncomplete;
	}

	return result;
}

	}
}
