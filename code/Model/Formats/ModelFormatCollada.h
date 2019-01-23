/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_model_ModelFormatCollada_H
#define traktor_model_ModelFormatCollada_H

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

/*! \brief Collada model format.
 * \ingroup Model
 */
class T_DLLCLASS ModelFormatCollada : public ModelFormat
{
	T_RTTI_CLASS;

public:
	virtual void getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const override final;

	virtual bool supportFormat(const std::wstring& extension) const override final;

	virtual Ref< Model > read(const Path& filePath, uint32_t importFlags, const std::function< Ref< IStream >(const Path&) >& openStream) const override final;

	virtual bool write(IStream* stream, const Model* model) const override final;
};

	}
}

#endif	// traktor_model_ModelFormatCollada_H
