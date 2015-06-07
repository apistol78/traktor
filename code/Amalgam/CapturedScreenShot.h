#ifndef traktor_amalgam_CapturedScreenShot_H
#define traktor_amalgam_CapturedScreenShot_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief Captured screenshot response from running target.
 * \ingroup Amalgam
 */
class T_DLLCLASS CapturedScreenShot : public ISerializable
{
	T_RTTI_CLASS;

public:
	CapturedScreenShot();

	CapturedScreenShot(int32_t width, int32_t height);

	virtual void serialize(ISerializer& s);

	int32_t getWidth() const { return m_width; }

	int32_t getHeight() const { return m_height; }

	AlignedVector< uint32_t >& getData() { return m_data; }

	const AlignedVector< uint32_t >& getData() const { return m_data; }

private:
	int32_t m_width;
	int32_t m_height;
	AlignedVector< uint32_t > m_data;
};

	}
}

#endif	// traktor_amalgam_CapturedScreenShot_H
