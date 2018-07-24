/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#pragma once

#include <vector>
#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{

class Path;
class IStream;
class Vector4;

	namespace animation
	{

class Skeleton;

/*! \brief
 * \ingroup Animation
 */
class SkeletonFormat : public Object
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
	 * \param extension Model extension.
	 * \return True if format is supported.
	 */
	virtual bool supportFormat(const std::wstring& extension) const = 0;

	/*! \brief Read skeleton.
	 *
	 * \param stream Source stream.
	 * \param offset Skeleton offset.
	 * \param scale Skeleton scale.
	 * \param radius Skeleton radius.
	 * \param invertX Invert X coordinates.
	 * \param invertZ Invert Z coordinates.
	 * \return Read skeleton.
	 */
	virtual Ref< Skeleton > read(IStream* stream, const Vector4& offset, float scale, float radius, bool invertX, bool invertZ) const = 0;

	/*! \brief Automatically read skeleton using appropriate format.
	 *
	 * \param filePath Path to skeleton file.
	 * \param offset Skeleton offset.
	 * \param scale Skeleton scale.
	 * \param radius Skeleton radius.
	 * \param invertX Invert X coordinates.
	 * \param invertZ Invert Z coordinates.
	 * \return Read skeleton.
	 */
	static Ref< Skeleton > readAny(const Path& filePath, const Vector4& offset, float scale, float radius, bool invertX, bool invertZ);

	/*! \brief Automatically read skeleton using appropriate format.
	 *
	 * \param file Source stream.
	 * \param extension File format extension.
	 * \param offset Skeleton offset.
	 * \param scale Skeleton scale.
	 * \param radius Skeleton radius.
	 * \param invertX Invert X coordinates.
	 * \param invertZ Invert Z coordinates.
	 * \return Read skeleton.
	 */
	static Ref< Skeleton > readAny(IStream* stream, const std::wstring& extension, const Vector4& offset, float scale, float radius, bool invertX, bool invertZ);
};

	}
}
