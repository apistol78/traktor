/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/Object.h"
//#include "Core/Heap/Ref.h"
#include "Core/RefArray.h"

namespace traktor
{

class A : public Object {};

class B : public A {};

class C : public Object {};

struct DummyPred { bool operator () (A* a) const { return false; } };

void referenceTest()
{
	RefArray< A > ra;
//	Ref< A > a;
//	A* a;
	
//	*ra.begin() = a;
	
	std::remove_if(ra.begin(), ra.end(), DummyPred());
}

//// OK
//Ref< A > staticImplicitCast_1()
//{
//	Ref< B > ref_B;
//	return ref_B;
//}
//
//// OK
//Ref< A > staticImplicitCast_2()
//{
//	RefArray< B > refArray_B;
//	return refArray_B[0];
//}
//
//bool sortPredicate(const A* left, const A* right)
//{
//	return false;
//}
//
//// Not OK
////Ref< A > staticImplicitCast_3()
////{
////	RefArray< C > refArray_C;
////	return refArray_C[0];
////}
//
//void staticRefCase()
//{
//	A* direct_ptr_A = 0;
//	B* direct_ptr_B = 0;
//	const A* const_direct_ptr_A = 0;
//	const B* const_direct_ptr_B = 0;
//
//	Ref< A > ref_A_1 = direct_ptr_A;		// OK
////	Ref< A > ref_A_2 = const_direct_ptr_A;	// Not OK
//	Ref< A > ref_A_3 = direct_ptr_B;		// Ok
////	Ref< A > ref_A_4 = const_direct_ptr_B;	// Not OK
//
////	Ref< B > ref_B_1 = direct_ptr_A;		// Not OK
//	Ref< B > ref_B_2 = direct_ptr_B;		// OK
////	Ref< B > ref_B_3 = const_direct_ptr_A;	// Not OK
////	Ref< B > ref_B_4 = const_direct_ptr_B;	// Not OK
//
//	Ref< const A > const_ref_A_1 = direct_ptr_A;		// OK
//	Ref< const A > const_ref_A_2 = direct_ptr_B;		// OK
//	Ref< const A > const_ref_A_3 = const_direct_ptr_A;	// OK
//	Ref< const A > const_ref_A_4 = const_direct_ptr_B;	// OK
//
////	Ref< const B > const_ref_B_1 = direct_ptr_A;		// Not OK
//	Ref< const B > const_ref_B_2 = direct_ptr_B;		// OK
////	Ref< const B > const_ref_B_3 = const_direct_ptr_A;	// Not OK
//	Ref< const B > const_ref_B_4 = const_direct_ptr_B;	// OK
//
//	ref_A_1 = ref_A_3;	// OK
//	ref_A_1 = ref_B_2;	// OK
////	ref_B_2 = ref_A_1;	// Not OK
//
////	ref_A_1 = const_ref_A_1;		// Not OK
//	const_ref_A_1 = ref_A_1;		// OK
//
////	ref_A_1 = const_ref_B_2;		// Not OK
//	const_ref_A_1 = ref_B_2;		// OK
//
//	const_ref_A_1 = const_ref_B_2;	// OK
////	const_ref_B_2 = const_ref_A_1;	// Not OK
//
//
//	RefArray< B > refArray_B;
//	ref_A_1 = refArray_B[0];	// OK
//	ref_B_2 = refArray_B[0];	// OK
////	refArray_B[0] = ref_A_1;	// Not OK
//	refArray_B[0] = ref_B_2;	// OK
//
//	direct_ptr_A = refArray_B[0];	// OK
//	direct_ptr_B = refArray_B[0];	// OK
////	refArray_B[0] = direct_ptr_A;	// Not OK
//	refArray_B[0] = direct_ptr_B;	// OK
//
//
//	RefArray< A > ra_1;
//	RefArray< A > ra_2(100);
//	RefArray< A > ra_3(ra_2);
//
//	for (RefArray< A >::iterator i = ra_2.begin(); i != ra_2.end(); ++i)
//		;
//
//	for (RefArray< A >::const_iterator i = ra_2.begin(); i != ra_2.end(); ++i)
//		;
//
//	std::reverse(ra_2.begin(), ra_2.end());
//	std::sort(ra_2.begin(), ra_2.end(), sortPredicate);
//}

}
