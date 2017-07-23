/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef AggregationItem_H
#define AggregationItem_H

#include <Core/Serialization/ISerializable.h>

class AggregationItem : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	/*! \brief Set source file path.
	 *
	 * \param sourceFile Source path.
	 */
	void setSourceFile(const std::wstring& sourceFile);

	/*! \brief Get source file path.
	 */
	const std::wstring& getSourceFile() const;

	/*! \brief Set target path.
	 *
	 * Target path is relative to aggregation root;
	 * for instance the Xcode generated aggregation
	 * is the bundle root.
	 *
	 * \param targetPath Target path.
	 */
	void setTargetPath(const std::wstring& targetPath);

	/*! \brief Get target file path.
	 */
	const std::wstring& getTargetPath() const;

	virtual void serialize(traktor::ISerializer& s);

private:
	std::wstring m_sourceFile;
	std::wstring m_targetPath;
};

#endif	// AggregationItem_H
