#ifndef traktor_model_ImportFormatObj_H
#define traktor_model_ImportFormatObj_H

#include "Model/Import/ImportFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

/*! \brief Wavefront object model format.
 * \ingroup Model
 */
class T_DLLCLASS ImportFormatObj : public ImportFormat
{
	T_RTTI_CLASS(ImportFormatObj)

public:
	virtual void getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const;

	virtual bool supportFormat(const Path& filePath) const;

	virtual model::Model* import(const Path& filePath, uint32_t importFlags) const;
};

	}
}

#endif	// traktor_model_ImportFormatObj_H
