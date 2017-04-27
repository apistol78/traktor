/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "UnitTest/CasePath.h"
#include "Core/Io/Path.h"

namespace traktor
{

void CasePath::run()
{
	Path path1(L"c:/foo/bar.txt");
	Path path2(L"/foo/bar.txt");
	Path path3(L"foo/bar.txt");
	Path path4(L"c:/foo.txt");
	Path path5(L"/foo.txt");
	Path path6(L"foo.txt");

	CASE_ASSERT_EQUAL (path1.hasVolume(), true);
	CASE_ASSERT_EQUAL (path2.hasVolume(), false);
	CASE_ASSERT_EQUAL (path3.hasVolume(), false);
	CASE_ASSERT_EQUAL (path4.hasVolume(), true);
	CASE_ASSERT_EQUAL (path5.hasVolume(), false);
	CASE_ASSERT_EQUAL (path6.hasVolume(), false);

	CASE_ASSERT_EQUAL (path1.getVolume(), L"c");
	CASE_ASSERT_EQUAL (path2.getVolume(), L"");
	CASE_ASSERT_EQUAL (path3.getVolume(), L"");
	CASE_ASSERT_EQUAL (path4.getVolume(), L"c");
	CASE_ASSERT_EQUAL (path5.getVolume(), L"");
	CASE_ASSERT_EQUAL (path6.getVolume(), L"");

	CASE_ASSERT_EQUAL (path1.getFileName(), L"bar.txt");
	CASE_ASSERT_EQUAL (path2.getFileName(), L"bar.txt");
	CASE_ASSERT_EQUAL (path3.getFileName(), L"bar.txt");
	CASE_ASSERT_EQUAL (path4.getFileName(), L"foo.txt");
	CASE_ASSERT_EQUAL (path5.getFileName(), L"foo.txt");
	CASE_ASSERT_EQUAL (path6.getFileName(), L"foo.txt");

	CASE_ASSERT_EQUAL (path1.getFileNameNoExtension(), L"bar");
	CASE_ASSERT_EQUAL (path2.getFileNameNoExtension(), L"bar");
	CASE_ASSERT_EQUAL (path3.getFileNameNoExtension(), L"bar");
	CASE_ASSERT_EQUAL (path4.getFileNameNoExtension(), L"foo");
	CASE_ASSERT_EQUAL (path5.getFileNameNoExtension(), L"foo");
	CASE_ASSERT_EQUAL (path6.getFileNameNoExtension(), L"foo");

	CASE_ASSERT_EQUAL (path1.getPathOnly(), L"c:/foo");
	CASE_ASSERT_EQUAL (path2.getPathOnly(), L"/foo");
	CASE_ASSERT_EQUAL (path3.getPathOnly(), L"foo");
	CASE_ASSERT_EQUAL (path4.getPathOnly(), L"c:/");
	CASE_ASSERT_EQUAL (path5.getPathOnly(), L"/");
	CASE_ASSERT_EQUAL (path6.getPathOnly(), L"");

	CASE_ASSERT_EQUAL (path1.getPathName(), L"c:/foo/bar.txt");
	CASE_ASSERT_EQUAL (path2.getPathName(), L"/foo/bar.txt");
	CASE_ASSERT_EQUAL (path3.getPathName(), L"foo/bar.txt");
	CASE_ASSERT_EQUAL (path4.getPathName(), L"c:/foo.txt");
	CASE_ASSERT_EQUAL (path5.getPathName(), L"/foo.txt");
	CASE_ASSERT_EQUAL (path6.getPathName(), L"foo.txt");

	CASE_ASSERT_EQUAL (path1.getPathNameNoExtension(), L"c:/foo/bar");
	CASE_ASSERT_EQUAL (path2.getPathNameNoExtension(), L"/foo/bar");
	CASE_ASSERT_EQUAL (path3.getPathNameNoExtension(), L"foo/bar");
	CASE_ASSERT_EQUAL (path4.getPathNameNoExtension(), L"c:/foo");
	CASE_ASSERT_EQUAL (path5.getPathNameNoExtension(), L"/foo");
	CASE_ASSERT_EQUAL (path6.getPathNameNoExtension(), L"foo");

	CASE_ASSERT_EQUAL (path1.getPathNameNoVolume(), L"/foo/bar.txt");
	CASE_ASSERT_EQUAL (path2.getPathNameNoVolume(), L"/foo/bar.txt");
	CASE_ASSERT_EQUAL (path3.getPathNameNoVolume(), L"foo/bar.txt");
	CASE_ASSERT_EQUAL (path4.getPathNameNoVolume(), L"/foo.txt");
	CASE_ASSERT_EQUAL (path5.getPathNameNoVolume(), L"/foo.txt");
	CASE_ASSERT_EQUAL (path6.getPathNameNoVolume(), L"foo.txt");

	CASE_ASSERT_EQUAL (path1.getExtension(), L"txt");
	CASE_ASSERT_EQUAL (path2.getExtension(), L"txt");
	CASE_ASSERT_EQUAL (path3.getExtension(), L"txt");
	CASE_ASSERT_EQUAL (path4.getExtension(), L"txt");
	CASE_ASSERT_EQUAL (path5.getExtension(), L"txt");
	CASE_ASSERT_EQUAL (path6.getExtension(), L"txt");

	Path path7(L"c:/foo/../bar.txt");
	Path path8(L"c:/../bar.txt");

	CASE_ASSERT_EQUAL (path7.normalized().getPathName(), L"c:/bar.txt");
	CASE_ASSERT_EQUAL (path8.normalized().getPathName(), L"c:/../bar.txt");
}

}
