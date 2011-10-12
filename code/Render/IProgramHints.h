#ifndef traktor_render_IProgramHints_H
#define traktor_render_IProgramHints_H

#include "Core/Object.h"

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

/*! \brief Program compiler hints interface.
 * \ingroup Render
 */
class T_DLLCLASS IProgramHints : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Calculate parameter position, or constant index.
	 *
	 * Used for cross program locality so same parameter using in
	 * several programs are placed at the same location.
	 *
	 * \param name Name of parameter.
	 * \param size Size of parameter in number of elements.
	 * \param maxPosition Maximum position index (inclusive).
	 * \return Preferred position of parameter.
	 */
	virtual uint32_t getParameterPosition(const std::wstring& name, uint32_t size, uint32_t maxPosition) = 0;
};

	}
}

#endif	// traktor_render_IProgramHints_H
