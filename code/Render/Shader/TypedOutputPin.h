/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_TypedOutputPin_H
#define traktor_render_TypedOutputPin_H

#include <string>
#include "Render/Types.h"
#include "Render/Shader/OutputPin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Typed shader graph node output pin.
 * \ingroup Render
 */
class T_DLLCLASS TypedOutputPin : public OutputPin
{
public:
	TypedOutputPin();

	TypedOutputPin(Node* node, const std::wstring& name, ParameterType type);

	ParameterType getType() const;

private:
	ParameterType m_type;
};

	}
}

#endif	// traktor_render_TypedOutputPin_H
