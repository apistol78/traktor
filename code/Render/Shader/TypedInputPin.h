#ifndef traktor_render_TypedInputPin_H
#define traktor_render_TypedInputPin_H

#include <string>
#include "Render/Types.h"
#include "Render/Shader/InputPin.h"

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

/*! \brief Typed shader graph node input pin.
 * \ingroup Render
 */
class T_DLLCLASS TypedInputPin : public InputPin
{
public:
	TypedInputPin(Node* node, const std::wstring& name, bool optional, ParameterType type, const std::wstring& samplerId = L"");

	ParameterType getType() const;

	const std::wstring& getSamplerId() const;

private:
	ParameterType m_type;
	std::wstring m_samplerId;
};

	}
}

#endif	// traktor_render_TypedInputPin_H
