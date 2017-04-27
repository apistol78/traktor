/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_FlashEmptyMovieAsset_H
#define traktor_flash_FlashEmptyMovieAsset_H

#include "Core/Math/Color4ub.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class T_DLLCLASS FlashEmptyMovieAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	FlashEmptyMovieAsset();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	int32_t getStageWidth() const { return m_stageWidth; }

	int32_t getStageHeight() const { return m_stageHeight; }

	int32_t getFrameRate() const { return m_frameRate; }

	const Color4ub& getBackgroundColor() const { return m_backgroundColor; }

private:
	int32_t m_stageWidth;
	int32_t m_stageHeight;
	int32_t m_frameRate;
	Color4ub m_backgroundColor;
};

	}
}

#endif	// traktor_flash_FlashEmptyMovieAsset_H
