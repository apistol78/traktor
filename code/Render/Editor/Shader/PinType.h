#ifndef traktor_render_PinType_H
#define traktor_render_PinType_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Pin type class.
 * \ingroup Render
 */
enum PinTypeClass
{
	PtcVoid,
	PtcScalar,
	PtcMatrix,
	PtcTexture
};

/*! \brief Pin type.
 * \ingroup Render
 */
class T_DLLCLASS PinType : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Construct void pin type. */
	PinType();

	/*! \brief Construct pin type.
	 *
	 * \param ptc Pin type class.
	 * \param width Width of type; only reasonable for scalar class.
	 */
	PinType(PinTypeClass class_, int32_t width = 1);

	/*! \brief Pin type class.
	 *
	 * \return Pin type class.
	 */
	PinTypeClass getClass() const;

	/*! \brief Width of type.
	 *
	 * \return Width of type.
	 */
	int32_t getWidth() const;

	/*! \brief Return type with highest precedence.
	 *
	 * \param type1 First type.
	 * \param type2 Second type.
	 * \return Type with highest precedence.
	 */
	static PinType maxPrecedence(const PinType& type1, const PinType& type2);

private:
	PinTypeClass m_class;
	int32_t m_width;
};

	}
}

#endif	// traktor_render_PinType_H
