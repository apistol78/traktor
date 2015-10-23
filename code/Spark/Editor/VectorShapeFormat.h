#ifndef traktor_spark_VectorShapeFormat_H
#define traktor_spark_VectorShapeFormat_H

#include "Core/Object.h"

namespace traktor
{

class IStream;

	namespace spark
	{

class VectorShape;

/*! \brief Vector shape importer format.
 * \ingroup Spark
 *
 * VectorShapeFormat is used to import shapes
 * information from 3rd-party formats.
 */
class VectorShapeFormat : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Get list of common extensions of implemented format.
	 *
	 * \param outDescription Human readable description of format.
	 * \param outExtensions List of file extensions commonly used for format.
	 */
	virtual void getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const = 0;

	/*! \brief Determine if format support parsing file.
	 *
	 * \param extension Shape extension.
	 * \return True if format is supported.
	 */
	virtual bool supportFormat(const std::wstring& extension) const = 0;

	/*! \brief Read shape.
	 *
	 * \param stream Source stream.
	 * \return Read shape.
	 */
	virtual Ref< VectorShape > read(IStream* stream) const = 0;

	/*! \brief Automatically read shape using appropriate format.
	 *
	 * \param filePath Path to shape file.
	 * \return Read shape.
	 */
	static Ref< VectorShape > readAny(const Path& filePath);

	/*! \brief Automatically read shape using appropriate format.
	 *
	 * \param file Source stream.
	 * \param extension File format extension.
	 * \return Read shape.
	 */
	static Ref< VectorShape > readAny(IStream* stream, const std::wstring& extension);
};

	}
}

#endif	// traktor_spark_VectorShapeFormat_H
