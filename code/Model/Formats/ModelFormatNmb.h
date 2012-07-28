#ifndef traktor_model_ModelFormatNmb_H
#define traktor_model_ModelFormatNmb_H

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

/*! \brief Nvidia object model format.
 * \ingroup Model
 */
class T_DLLCLASS ModelFormatNmb : public ModelFormat
{
	T_RTTI_CLASS;

public:
	virtual void getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const;

	virtual bool supportFormat(const Path& filePath) const;

	virtual Ref< Model > read(const Path& filePath, uint32_t importFlags) const;

	virtual bool write(const Path& filePath, const Model* model) const;
};

	}
}

#endif	// traktor_model_ModelFormatNmb_H
