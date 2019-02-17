#pragma once

#include "Model/ModelFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

/*! \brief Wavefront object model format.
 * \ingroup Model
 */
class T_DLLCLASS ModelFormatObj : public ModelFormat
{
	T_RTTI_CLASS;

public:
	virtual void getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const override final;

	virtual bool supportFormat(const std::wstring& extension) const override final;

	virtual Ref< Model > read(const Path& filePath, const std::function< Ref< IStream >(const Path&) >& openStream) const override final;

	virtual bool write(IStream* stream, const Model* model) const override final;
};

	}
}
