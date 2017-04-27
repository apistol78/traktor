/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drawing_Config_H
#define traktor_drawing_Config_H

/*! \ingroup Drawing */
//@{

#if defined(_MSC_VER)
#	define DRAWING_INCLUDE_PNG
#	define DRAWING_INCLUDE_ICO
#	define DRAWING_INCLUDE_JPEG
#	define DRAWING_INCLUDE_GIF
#	if !defined(WINCE)
#		define DRAWING_INCLUDE_EXR
#	endif
#elif defined(__APPLE__)
#	define DRAWING_INCLUDE_PNG
#	define DRAWING_INCLUDE_ICO
#	define DRAWING_INCLUDE_JPEG
#	define DRAWING_INCLUDE_GIF
//#	define DRAWING_INCLUDE_EXR
#elif defined(__GNUC__)
#	define DRAWING_INCLUDE_PNG
#	define DRAWING_INCLUDE_ICO
#	define DRAWING_INCLUDE_JPEG
#	define DRAWING_INCLUDE_GIF
//#	define DRAWING_INCLUDE_EXR
#endif

// Santiy check
#if !defined(DRAWING_INCLUDE_PNG) && defined(DRAWING_INCLUDE_ICO)
#	pragma message("DRAWING_INCLUDE_ICO require DRAWING_INCLUDE_PNG")
#	undef DRAWING_INCLUDE_ICO
#endif

#if defined(_DEBUG)
#	define DRAWING_CHECK_DATA
#endif

//@}

#endif	// traktor_drawing_Config_H
