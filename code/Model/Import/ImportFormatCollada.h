#ifndef traktor_model_ImportFormatCollada_H
#define traktor_model_ImportFormatCollada_H

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

/*! \brief Collada model format.
 * \ingroup Model
 */
class T_DLLCLASS ImportFormatCollada : public ImportFormat
{
	T_RTTI_CLASS(ImportFormatCollada)

public:
	virtual void getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const;

	virtual bool supportFormat(const Path& filePath) const;

	virtual model::Model* import(const Path& filePath, uint32_t importFlags) const;

};

	}
}

#endif	// traktor_model_ImportFormatCollada_H
