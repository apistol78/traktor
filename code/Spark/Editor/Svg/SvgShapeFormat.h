#ifndef traktor_spark_SvgShapeFormat_H
#define traktor_spark_SvgShapeFormat_H

#include "Spark/Editor/VectorShapeFormat.h"

namespace traktor
{
	namespace spark
	{

/*! \brief
 * \ingroup Spark
 */
class SvgShapeFormat : public VectorShapeFormat
{
	T_RTTI_CLASS;

public:
	virtual void getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const T_OVERRIDE T_FINAL;

	virtual bool supportFormat(const std::wstring& extension) const T_OVERRIDE T_FINAL;

	virtual Ref< VectorShape > read(IStream* stream) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_spark_SvgShapeFormat_H
