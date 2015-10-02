#ifndef traktor_spark_Character_H
#define traktor_spark_Character_H

#include "Core/Math/Matrix33.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

/*! \brief Character base class.
 * \ingroup Spark
 */
class T_DLLCLASS Character : public ISerializable
{
	T_RTTI_CLASS;

public:
	Character();

	/*! \brief Set local transform.
	 */
	void setTransform(const Matrix33& transform);

	/*! \brief Get local transform.
	 */
	const Matrix33& getTransform() const;

	virtual void serialize(ISerializer& s);

private:
	Matrix33 m_transform;
};

	}
}

#endif	// traktor_spark_Character_H
