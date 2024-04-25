/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/BoxedClassFactory.h"
#include "Core/Class/IRuntimeClassRegistrar.h"

// Boxes
#include "Core/Class/Boxes/BoxedAabb2.h"
#include "Core/Class/Boxes/BoxedAabb3.h"
#include "Core/Class/Boxes/BoxedAlignedVector.h"
#include "Core/Class/Boxes/BoxedBezier2nd.h"
#include "Core/Class/Boxes/BoxedBezier3rd.h"
#include "Core/Class/Boxes/BoxedColor4f.h"
#include "Core/Class/Boxes/BoxedColor4ub.h"
#include "Core/Class/Boxes/BoxedFrustum.h"
#include "Core/Class/Boxes/BoxedGuid.h"
#include "Core/Class/Boxes/BoxedIntervalTransform.h"
#include "Core/Class/Boxes/BoxedMatrix33.h"
#include "Core/Class/Boxes/BoxedMatrix44.h"
#include "Core/Class/Boxes/BoxedPlane.h"
#include "Core/Class/Boxes/BoxedPointer.h"
#include "Core/Class/Boxes/BoxedQuaternion.h"
#include "Core/Class/Boxes/BoxedRandomGeometry.h"
#include "Core/Class/Boxes/BoxedRange.h"
#include "Core/Class/Boxes/BoxedRay3.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Class/Boxes/BoxedSphere.h"
#include "Core/Class/Boxes/BoxedStdVector.h"
#include "Core/Class/Boxes/BoxedTypeInfo.h"
#include "Core/Class/Boxes/BoxedVector4Array.h"
#include "Core/Class/Boxes/BoxedWinding2.h"
#include "Core/Class/Boxes/BoxedWinding3.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.BoxedClassFactory", 0, BoxedClassFactory, IRuntimeClassFactory)

void BoxedClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classBoxed = new AutoRuntimeClass< Boxed >();
	classBoxed->addMethod("toString", &Boxed::toString);
	registrar->registerClass(classBoxed);

	auto classBoxedAabb2 = new AutoRuntimeClass< BoxedAabb2 >();
	classBoxedAabb2->addConstructor();
	classBoxedAabb2->addConstructor< const BoxedVector2*, const BoxedVector2* >();
	classBoxedAabb2->addProperty("min", &BoxedAabb2::min);
	classBoxedAabb2->addProperty("max", &BoxedAabb2::max);
	classBoxedAabb2->addProperty("center", &BoxedAabb2::getCenter);
	classBoxedAabb2->addProperty("extent", &BoxedAabb2::getExtent);
	classBoxedAabb2->addProperty("empty", &BoxedAabb2::empty);
	classBoxedAabb2->addMethod("inside", &BoxedAabb2::inside);
	classBoxedAabb2->addMethod("contain", &BoxedAabb2::contain);
	registrar->registerClass(classBoxedAabb2);

	auto classBoxedAabb3 = new AutoRuntimeClass< BoxedAabb3 >();
	classBoxedAabb3->addConstructor();
	classBoxedAabb3->addConstructor< const BoxedVector4*, const BoxedVector4* >();
	classBoxedAabb3->addProperty("min", &BoxedAabb3::min);
	classBoxedAabb3->addProperty("max", &BoxedAabb3::max);
	classBoxedAabb3->addProperty("min", &BoxedAabb3::getMin);
	classBoxedAabb3->addProperty("max", &BoxedAabb3::getMax);
	classBoxedAabb3->addProperty("center", &BoxedAabb3::getCenter);
	classBoxedAabb3->addProperty("extent", &BoxedAabb3::getExtent);
	classBoxedAabb3->addProperty("empty", &BoxedAabb3::empty);
	classBoxedAabb3->addMethod("inside", &BoxedAabb3::inside);
	classBoxedAabb3->addMethod("contain", &BoxedAabb3::contain);
	classBoxedAabb3->addMethod("scale", &BoxedAabb3::scale);
	classBoxedAabb3->addMethod("expand", &BoxedAabb3::expand);
	classBoxedAabb3->addMethod("transform", &BoxedAabb3::transform);
	classBoxedAabb3->addMethod("overlap", &BoxedAabb3::overlap);
	classBoxedAabb3->addMethod("intersectRay", &BoxedAabb3::intersectRay);
	registrar->registerClass(classBoxedAabb3);

	auto classBoxedAlignedVector = new AutoRuntimeClass< BoxedAlignedVector >();
	classBoxedAlignedVector->addConstructor();
	classBoxedAlignedVector->addConstructor< uint32_t >();
	classBoxedAlignedVector->addProperty("size", &BoxedAlignedVector::size);
	classBoxedAlignedVector->addProperty("empty", &BoxedAlignedVector::empty);
	classBoxedAlignedVector->addProperty("front", &BoxedAlignedVector::front);
	classBoxedAlignedVector->addProperty("back", &BoxedAlignedVector::back);
	classBoxedAlignedVector->addMethod("reserve", &BoxedAlignedVector::reserve);
	classBoxedAlignedVector->addMethod("resize", &BoxedAlignedVector::resize);
	classBoxedAlignedVector->addMethod("clear", &BoxedAlignedVector::clear);
	classBoxedAlignedVector->addMethod("push_back", &BoxedAlignedVector::push_back);
	classBoxedAlignedVector->addMethod("pop_back", &BoxedAlignedVector::pop_back);
	classBoxedAlignedVector->addMethod("set", &BoxedAlignedVector::set);
	classBoxedAlignedVector->addMethod("get", &BoxedAlignedVector::get);
	registrar->registerClass(classBoxedAlignedVector);

	auto classBoxedBezier2nd = new AutoRuntimeClass< BoxedBezier2nd >();
	classBoxedBezier2nd->addConstructor();
	classBoxedBezier2nd->addConstructor< const BoxedVector2*, const BoxedVector2*, const BoxedVector2* >();
	classBoxedBezier2nd->addProperty("cp0", &BoxedBezier2nd::cp0);
	classBoxedBezier2nd->addProperty("cp1", &BoxedBezier2nd::cp1);
	classBoxedBezier2nd->addProperty("cp2", &BoxedBezier2nd::cp2);
	classBoxedBezier2nd->addMethod("evaluate", &BoxedBezier2nd::evaluate);
	classBoxedBezier2nd->addMethod("tangent", &BoxedBezier2nd::tangent);
	classBoxedBezier2nd->addMethod("getLocalMinMaxY", &BoxedBezier2nd::getLocalMinMaxY);
	classBoxedBezier2nd->addMethod("getLocalMinMaxX", &BoxedBezier2nd::getLocalMinMaxX);
	classBoxedBezier2nd->addMethod("intersectX", &BoxedBezier2nd::intersectX);
	classBoxedBezier2nd->addMethod("intersectY", &BoxedBezier2nd::intersectY);
	classBoxedBezier2nd->addMethod("split", &BoxedBezier2nd::split);
	classBoxedBezier2nd->addMethod("toBezier3rd", &BoxedBezier2nd::toBezier3rd);
	registrar->registerClass(classBoxedBezier2nd);

	auto classBoxedBezier3rd = new AutoRuntimeClass< BoxedBezier3rd >();
	classBoxedBezier3rd->addConstructor();
	classBoxedBezier3rd->addConstructor< const BoxedVector2*, const BoxedVector2*, const BoxedVector2*, const BoxedVector2* >();
	classBoxedBezier3rd->addProperty("cp0", &BoxedBezier3rd::cp0);
	classBoxedBezier3rd->addProperty("cp1", &BoxedBezier3rd::cp1);
	classBoxedBezier3rd->addProperty("cp2", &BoxedBezier3rd::cp2);
	classBoxedBezier3rd->addProperty("cp3", &BoxedBezier3rd::cp3);
	classBoxedBezier3rd->addProperty("flatness", &BoxedBezier3rd::flatness);
	classBoxedBezier3rd->addMethod("evaluate", &BoxedBezier3rd::evaluate);
	classBoxedBezier3rd->addMethod("tangent", &BoxedBezier3rd::tangent);
	classBoxedBezier3rd->addMethod("isFlat", &BoxedBezier3rd::isFlat);
	classBoxedBezier3rd->addMethod("split", &BoxedBezier3rd::split);
	classBoxedBezier3rd->addMethod("approximate", &BoxedBezier3rd::approximate);
	registrar->registerClass(classBoxedBezier3rd);

	auto classBoxedColor4f = new AutoRuntimeClass< BoxedColor4f >();
	classBoxedColor4f->addConstructor();
	classBoxedColor4f->addConstructor< float, float, float >();
	classBoxedColor4f->addConstructor< float, float, float, float >();
	classBoxedColor4f->addProperty("red", &BoxedColor4f::setRed, &BoxedColor4f::getRed);
	classBoxedColor4f->addProperty("green", &BoxedColor4f::setGreen, &BoxedColor4f::getGreen);
	classBoxedColor4f->addProperty("blue", &BoxedColor4f::setBlue, &BoxedColor4f::getBlue);
	classBoxedColor4f->addProperty("alpha", &BoxedColor4f::setAlpha, &BoxedColor4f::getAlpha);
	classBoxedColor4f->addProperty("rgb0", &BoxedColor4f::rgb0);
	classBoxedColor4f->addProperty("rgb1", &BoxedColor4f::rgb1);
	classBoxedColor4f->addProperty("aaa0", &BoxedColor4f::aaa0);
	classBoxedColor4f->addProperty("aaa1", &BoxedColor4f::aaa1);
	classBoxedColor4f->addProperty("aaaa", &BoxedColor4f::aaaa);
	classBoxedColor4f->addMethod("get", &BoxedColor4f::get);
	classBoxedColor4f->addMethod("set", &BoxedColor4f::set);
	classBoxedColor4f->addStaticMethod("lerp", &BoxedColor4f::lerp);
	classBoxedColor4f->addOperator< Color4f, const BoxedColor4f* >('+', &BoxedColor4f::add);
	classBoxedColor4f->addOperator< Color4f, const BoxedColor4f* >('-', &BoxedColor4f::sub);
	classBoxedColor4f->addOperator< Color4f, const BoxedColor4f* >('*', &BoxedColor4f::mul);
	classBoxedColor4f->addOperator< Color4f, float >('*', &BoxedColor4f::mul);
	classBoxedColor4f->addOperator< Color4f, const BoxedColor4f* >('/', &BoxedColor4f::div);
	classBoxedColor4f->addOperator< Color4f, float >('/', &BoxedColor4f::div);
	registrar->registerClass(classBoxedColor4f);

	auto classBoxedColor4ub = new AutoRuntimeClass< BoxedColor4ub >();
	classBoxedColor4ub->addConstructor();
	classBoxedColor4ub->addConstructor< uint8_t, uint8_t, uint8_t >();
	classBoxedColor4ub->addConstructor< uint8_t, uint8_t, uint8_t, uint8_t >();
	classBoxedColor4ub->addProperty("red", &BoxedColor4ub::setRed, &BoxedColor4ub::getRed);
	classBoxedColor4ub->addProperty("green", &BoxedColor4ub::setGreen, &BoxedColor4ub::getGreen);
	classBoxedColor4ub->addProperty("blue", &BoxedColor4ub::setBlue, &BoxedColor4ub::getBlue);
	classBoxedColor4ub->addProperty("alpha", &BoxedColor4ub::setAlpha, &BoxedColor4ub::getAlpha);
	classBoxedColor4ub->addMethod("getARGB", &BoxedColor4ub::getARGB);
	classBoxedColor4ub->addMethod("formatRGB", &BoxedColor4ub::formatRGB);
	classBoxedColor4ub->addMethod("formatARGB", &BoxedColor4ub::formatARGB);
	registrar->registerClass(classBoxedColor4ub);

	auto classBoxedFrustum = new AutoRuntimeClass< BoxedFrustum >();
	classBoxedFrustum->addConstructor();
	classBoxedFrustum->addProperty("nearZ", &BoxedFrustum::setNearZ, &BoxedFrustum::getNearZ);
	classBoxedFrustum->addProperty("farZ", &BoxedFrustum::setFarZ, &BoxedFrustum::getFarZ);
	classBoxedFrustum->addMethod("buildPerspective", &BoxedFrustum::buildPerspective);
	classBoxedFrustum->addMethod("buildOrtho", &BoxedFrustum::buildOrtho);
	classBoxedFrustum->addMethod("insidePoint", &BoxedFrustum::insidePoint);
	classBoxedFrustum->addMethod("insideSphere", &BoxedFrustum::insideSphere);
	classBoxedFrustum->addMethod("insideAabb", &BoxedFrustum::insideAabb);
	classBoxedFrustum->addMethod("getPlane", &BoxedFrustum::getPlane);
	classBoxedFrustum->addMethod("getCorner", &BoxedFrustum::getCorner);
	classBoxedFrustum->addMethod("getCenter", &BoxedFrustum::getCenter);
	registrar->registerClass(classBoxedFrustum);

	auto classBoxedGuid = new AutoRuntimeClass< BoxedGuid >();
	classBoxedGuid->addConstructor();
	classBoxedGuid->addConstructor< const std::wstring& >();
	classBoxedGuid->addStaticMethod("create", &BoxedGuid::create);
	classBoxedGuid->addMethod("set", &BoxedGuid::set);
	classBoxedGuid->addMethod("format", &BoxedGuid::format);
	classBoxedGuid->addMethod("isValid", &BoxedGuid::isValid);
	classBoxedGuid->addMethod("isNull", &BoxedGuid::isNull);
	classBoxedGuid->addMethod("isNotNull", &BoxedGuid::isNotNull);
	registrar->registerClass(classBoxedGuid);

	auto classBoxedIntervalTransform = new AutoRuntimeClass< BoxedIntervalTransform >();
	classBoxedIntervalTransform->addConstructor();
	classBoxedIntervalTransform->addMethod("get", &BoxedIntervalTransform::get);
	registrar->registerClass(classBoxedIntervalTransform);

	auto classBoxedMatrix33 = new AutoRuntimeClass< BoxedMatrix33 >();
	classBoxedMatrix33->addConstructor();
	classBoxedMatrix33->addConstant("zero", CastAny< Matrix33 >::set(Matrix33::zero()));
	classBoxedMatrix33->addConstant("identity", CastAny< Matrix33 >::set(Matrix33::identity()));
	classBoxedMatrix33->addMethod("diagonal", &BoxedMatrix33::diagonal);
	classBoxedMatrix33->addMethod("determinant", &BoxedMatrix33::determinant);
	classBoxedMatrix33->addMethod("transpose", &BoxedMatrix33::transpose);
	classBoxedMatrix33->addMethod("inverse", &BoxedMatrix33::inverse);
	classBoxedMatrix33->addMethod("set", &BoxedMatrix33::set);
	classBoxedMatrix33->addMethod("get", &BoxedMatrix33::get);
	classBoxedMatrix33->addMethod("concat", &BoxedMatrix33::concat);
	classBoxedMatrix33->addMethod("transform", &BoxedMatrix33::transform);
	classBoxedMatrix33->addStaticMethod("translate", &BoxedMatrix33::translate);
	classBoxedMatrix33->addStaticMethod("scale", &BoxedMatrix33::scale);
	classBoxedMatrix33->addStaticMethod("rotate", &BoxedMatrix33::rotate);
	classBoxedMatrix33->addOperator< Vector2, const BoxedVector2* >('*', &BoxedMatrix33::transform);
	classBoxedMatrix33->addOperator< Matrix33, const BoxedMatrix33* >('*', &BoxedMatrix33::concat);
	registrar->registerClass(classBoxedMatrix33);

	auto classBoxedMatrix44 = new AutoRuntimeClass< BoxedMatrix44 >();
	classBoxedMatrix44->addConstructor();
	classBoxedMatrix44->addConstructor< const BoxedVector4*, const BoxedVector4*, const BoxedVector4*, const BoxedVector4* >();
	classBoxedMatrix44->addConstant("zero", CastAny< Matrix44 >::set(Matrix44::zero()));
	classBoxedMatrix44->addConstant("identity", CastAny< Matrix44 >::set(Matrix44::identity()));
	classBoxedMatrix44->addMethod("axisX", &BoxedMatrix44::axisX);
	classBoxedMatrix44->addMethod("axisY", &BoxedMatrix44::axisY);
	classBoxedMatrix44->addMethod("axisZ", &BoxedMatrix44::axisZ);
	classBoxedMatrix44->addMethod("planeX", &BoxedMatrix44::planeX);
	classBoxedMatrix44->addMethod("planeY", &BoxedMatrix44::planeY);
	classBoxedMatrix44->addMethod("planeZ", &BoxedMatrix44::planeZ);
	classBoxedMatrix44->addMethod("translation", &BoxedMatrix44::translation);
	classBoxedMatrix44->addMethod("diagonal", &BoxedMatrix44::diagonal);
	classBoxedMatrix44->addMethod("isOrtho", &BoxedMatrix44::isOrtho);
	classBoxedMatrix44->addMethod("determinant", &BoxedMatrix44::determinant);
	classBoxedMatrix44->addMethod("transpose", &BoxedMatrix44::transpose);
	classBoxedMatrix44->addMethod("inverse", &BoxedMatrix44::inverse);
	classBoxedMatrix44->addMethod("setColumn", &BoxedMatrix44::setColumn);
	classBoxedMatrix44->addMethod("getColumn", &BoxedMatrix44::getColumn);
	classBoxedMatrix44->addMethod("setRow", &BoxedMatrix44::setRow);
	classBoxedMatrix44->addMethod("getRow", &BoxedMatrix44::getRow);
	classBoxedMatrix44->addMethod("set", &BoxedMatrix44::set);
	classBoxedMatrix44->addMethod("get", &BoxedMatrix44::get);
	classBoxedMatrix44->addMethod("concat", &BoxedMatrix44::concat);
	classBoxedMatrix44->addMethod("transform", &BoxedMatrix44::transform);
	classBoxedMatrix44->addStaticMethod< Matrix44, const Vector4& >("translate", &translate);
	classBoxedMatrix44->addStaticMethod< Matrix44, float, float, float >("translate", &translate);
	classBoxedMatrix44->addStaticMethod("rotateX", &rotateX);
	classBoxedMatrix44->addStaticMethod("rotateY", &rotateY);
	classBoxedMatrix44->addStaticMethod("rotateZ", &rotateZ);
	classBoxedMatrix44->addStaticMethod< Matrix44, const Vector4& >("scale", &scale);
	classBoxedMatrix44->addStaticMethod< Matrix44, float, float, float >("scale", &scale);
	classBoxedMatrix44->addOperator< Vector4, const BoxedVector4* >('*', &BoxedMatrix44::transform);
	classBoxedMatrix44->addOperator< Matrix44, const BoxedMatrix44* >('*', &BoxedMatrix44::concat);
	registrar->registerClass(classBoxedMatrix44);

	auto classBoxedPlane = new AutoRuntimeClass< BoxedPlane >();
	classBoxedPlane->addConstructor();
	classBoxedPlane->addConstructor< const Plane& >();
	classBoxedPlane->addConstructor< const Vector4&, float >();
	classBoxedPlane->addConstructor< const Vector4&, const Vector4& >();
	classBoxedPlane->addConstructor< const Vector4&, const Vector4&, const Vector4& >();
	classBoxedPlane->addConstructor< float, float, float, float >();
	classBoxedPlane->addProperty("normal", &BoxedPlane::setNormal, &BoxedPlane::normal);
	classBoxedPlane->addProperty("distance", &BoxedPlane::setDistance, &BoxedPlane::distance);
	classBoxedPlane->addMethod("distanceToPoint", &BoxedPlane::distanceToPoint);
	classBoxedPlane->addMethod("project", &BoxedPlane::project);
	classBoxedPlane->addMethod("intersectRay", &BoxedPlane::intersectRay);
	classBoxedPlane->addMethod("intersectSegment", &BoxedPlane::intersectSegment);
	classBoxedPlane->addStaticMethod("uniqueIntersectionPoint", &BoxedPlane::uniqueIntersectionPoint);
	registrar->registerClass(classBoxedPlane);

	auto classBoxedPointer = new AutoRuntimeClass< BoxedPointer >();
	classBoxedPointer->addConstructor();
	classBoxedPointer->addProperty("null", &BoxedPointer::null);
	registrar->registerClass(classBoxedPointer);

	auto classBoxedQuaternion = new AutoRuntimeClass< BoxedQuaternion >();
	classBoxedQuaternion->addConstructor();
	classBoxedQuaternion->addConstructor< float, float, float, float >();
	classBoxedQuaternion->addConstructor< const BoxedVector4*, float >();
	classBoxedQuaternion->addConstructor< float, float, float >();
	classBoxedQuaternion->addConstructor< const BoxedVector4*, const BoxedVector4* >();
	classBoxedQuaternion->addConstructor< const BoxedMatrix44* >();
	classBoxedQuaternion->addConstant("identity", CastAny< Quaternion >::set(Quaternion::identity()));
	classBoxedQuaternion->addProperty< float >("x", &BoxedQuaternion::set_x, &BoxedQuaternion::get_x);
	classBoxedQuaternion->addProperty< float >("y", &BoxedQuaternion::set_y, &BoxedQuaternion::get_y);
	classBoxedQuaternion->addProperty< float >("x", &BoxedQuaternion::set_z, &BoxedQuaternion::get_z);
	classBoxedQuaternion->addProperty< float >("y", &BoxedQuaternion::set_w, &BoxedQuaternion::get_w);
	classBoxedQuaternion->addProperty("eulerAngles", &BoxedQuaternion::getEulerAngles);
	classBoxedQuaternion->addProperty("axisAngle", &BoxedQuaternion::getAxisAngle);
	classBoxedQuaternion->addMethod("normalized", &BoxedQuaternion::normalized);
	classBoxedQuaternion->addMethod("inverse", &BoxedQuaternion::inverse);
	classBoxedQuaternion->addMethod("concat", &BoxedQuaternion::concat);
	classBoxedQuaternion->addMethod("transform", &BoxedQuaternion::transform);
	classBoxedQuaternion->addStaticMethod("fromEulerAngles", &BoxedQuaternion::fromEulerAngles);
	classBoxedQuaternion->addStaticMethod("fromAxisAngle", &BoxedQuaternion::fromAxisAngle);
	classBoxedQuaternion->addStaticMethod("lerp", &BoxedQuaternion::lerp);
	classBoxedQuaternion->addStaticMethod("slerp", &BoxedQuaternion::slerp);
	classBoxedQuaternion->addOperator< Vector4, const BoxedVector4* >('*', &BoxedQuaternion::transform);
	classBoxedQuaternion->addOperator< Quaternion, const BoxedQuaternion* >('*', &BoxedQuaternion::concat);
	registrar->registerClass(classBoxedQuaternion);

	auto classBoxedRandom = new AutoRuntimeClass< BoxedRandom >();
	classBoxedRandom->addConstructor();
	classBoxedRandom->addConstructor< uint32_t >();
	classBoxedRandom->addMethod("next", &BoxedRandom::next);
	classBoxedRandom->addMethod("nextFloat", &BoxedRandom::nextFloat);
	registrar->registerClass(classBoxedRandom);

	auto classBoxedRandomGeometry = new AutoRuntimeClass< BoxedRandomGeometry >();
	classBoxedRandomGeometry->addConstructor();
	classBoxedRandomGeometry->addConstructor< uint32_t >();
	classBoxedRandomGeometry->addMethod("nextUnit", &BoxedRandomGeometry::nextUnit);
	classBoxedRandomGeometry->addMethod("nextHemi", &BoxedRandomGeometry::nextHemi);
	registrar->registerClass(classBoxedRandomGeometry);

	auto classBoxedRange = new AutoRuntimeClass< BoxedRange >();
	classBoxedRange->addConstructor();
	classBoxedRange->addProperty("min", &BoxedRange::min);
	classBoxedRange->addProperty("max", &BoxedRange::max);
	registrar->registerClass(classBoxedRange);

	auto classBoxedRay3 = new AutoRuntimeClass< BoxedRay3 >();
	classBoxedRay3->addConstructor();
	classBoxedRay3->addConstructor< const Ray3& >();
	classBoxedRay3->addProperty("origin", &BoxedRay3::origin);
	classBoxedRay3->addProperty("direction", &BoxedRay3::direction);
	classBoxedRay3->addMethod("distance", &BoxedRay3::distance);
	classBoxedRay3->addOperator< Vector4, float >('*', &BoxedRay3::mul);
	registrar->registerClass(classBoxedRay3);

	auto classBoxedRefArray = new AutoRuntimeClass< BoxedRefArray >();
	classBoxedRefArray->addConstructor();
	classBoxedRefArray->addConstructor< const RefArray< Object >& >();
	classBoxedRefArray->addProperty("size", &BoxedRefArray::size);
	classBoxedRefArray->addProperty("empty", &BoxedRefArray::empty);
	classBoxedRefArray->addProperty("front", &BoxedRefArray::front);
	classBoxedRefArray->addProperty("back", &BoxedRefArray::back);
	classBoxedRefArray->addMethod("set", &BoxedRefArray::set);
	classBoxedRefArray->addMethod("get", &BoxedRefArray::get);
	classBoxedRefArray->addMethod("push_back", &BoxedRefArray::push_back);
	classBoxedRefArray->addMethod("pop_back", &BoxedRefArray::pop_back);
	classBoxedRefArray->addMethod("remove", &BoxedRefArray::remove);
	registrar->registerClass(classBoxedRefArray);

	auto classBoxedSphere = new AutoRuntimeClass< BoxedSphere >();
	classBoxedSphere->addConstructor();
	classBoxedSphere->addConstructor< const Vector4&, float >();
	classBoxedSphere->addProperty("center", &BoxedSphere::setCenter, &BoxedSphere::center);
	classBoxedSphere->addProperty("radius", &BoxedSphere::setRadius, &BoxedSphere::radius);
	classBoxedSphere->addMethod("inside", &BoxedSphere::inside);
	classBoxedSphere->addMethod("intersectRay", &BoxedSphere::intersectRay);
	registrar->registerClass(classBoxedSphere);

	auto classBoxedStdVector = new AutoRuntimeClass< BoxedStdVector >();
	classBoxedStdVector->addConstructor();
	classBoxedStdVector->addConstructor< uint32_t >();
	classBoxedStdVector->addProperty("size", &BoxedStdVector::size);
	classBoxedStdVector->addProperty("empty", &BoxedStdVector::empty);
	classBoxedStdVector->addProperty("front", &BoxedStdVector::front);
	classBoxedStdVector->addProperty("back", &BoxedStdVector::back);
	classBoxedStdVector->addMethod("reserve", &BoxedStdVector::reserve);
	classBoxedStdVector->addMethod("resize", &BoxedStdVector::resize);
	classBoxedStdVector->addMethod("clear", &BoxedStdVector::clear);
	classBoxedStdVector->addMethod("push_back", &BoxedStdVector::push_back);
	classBoxedStdVector->addMethod("pop_back", &BoxedStdVector::pop_back);
	classBoxedStdVector->addMethod("set", &BoxedStdVector::set);
	classBoxedStdVector->addMethod("get", &BoxedStdVector::get);
	registrar->registerClass(classBoxedStdVector);

	auto classBoxedTransform = new AutoRuntimeClass< BoxedTransform >();
	classBoxedTransform->addConstructor();
	classBoxedTransform->addConstructor< const BoxedVector4*, const BoxedQuaternion* >();
	classBoxedTransform->addConstructor< const BoxedMatrix44* >();
	classBoxedTransform->addConstant("identity", CastAny< Transform >::set(Transform::identity()));
	classBoxedTransform->addProperty< const Vector4& >("translation", 0, &BoxedTransform::get_translation);
	classBoxedTransform->addProperty< const Quaternion& >("rotation", 0, &BoxedTransform::get_rotation);
	classBoxedTransform->addProperty< Vector4 >("axisX", &BoxedTransform::get_axisX);
	classBoxedTransform->addProperty< Vector4 >("axisY", &BoxedTransform::get_axisY);
	classBoxedTransform->addProperty< Vector4 >("axisZ", &BoxedTransform::get_axisZ);
	classBoxedTransform->addProperty< Plane >("planeX", &BoxedTransform::get_planeX);
	classBoxedTransform->addProperty< Plane >("planeY", &BoxedTransform::get_planeY);
	classBoxedTransform->addProperty< Plane >("planeZ", &BoxedTransform::get_planeZ);
	classBoxedTransform->addMethod("inverse", &BoxedTransform::inverse);
	classBoxedTransform->addMethod("toMatrix44", &BoxedTransform::toMatrix44);
	classBoxedTransform->addMethod("concat", &BoxedTransform::concat);
	classBoxedTransform->addMethod("transform", &BoxedTransform::transform);
	classBoxedTransform->addStaticMethod("lerp", &BoxedTransform::lerp);
	classBoxedTransform->addStaticMethod("lookAt", &BoxedTransform::lookAt);
	classBoxedTransform->addOperator< Vector4, const BoxedVector4* >('*', &BoxedTransform::transform);
	classBoxedTransform->addOperator< Transform, const BoxedTransform* >('*', &BoxedTransform::concat);
	registrar->registerClass(classBoxedTransform);

	auto classBoxedTypeInfo = new AutoRuntimeClass< BoxedTypeInfo >();
	classBoxedTypeInfo->addProperty("name", &BoxedTypeInfo::getName);
	classBoxedTypeInfo->addProperty("size", &BoxedTypeInfo::getSize);
	classBoxedTypeInfo->addProperty("version", &BoxedTypeInfo::getVersion);
	classBoxedTypeInfo->addMethod("createInstance", &BoxedTypeInfo::createInstance);
	classBoxedTypeInfo->addStaticMethod("find", &BoxedTypeInfo::find);
	classBoxedTypeInfo->addStaticMethod("findAllOf", &BoxedTypeInfo::findAllOf);
	registrar->registerClass(classBoxedTypeInfo);

	auto classBoxedVector2 = new AutoRuntimeClass< BoxedVector2 >();
	classBoxedVector2->addConstructor();
	classBoxedVector2->addConstructor< float, float >();
	classBoxedVector2->addConstant("zero", CastAny< Vector2 >::set(Vector2::zero()));
	classBoxedVector2->addProperty< float >("x", &BoxedVector2::set_x, &BoxedVector2::get_x);
	classBoxedVector2->addProperty< float >("y", &BoxedVector2::set_y, &BoxedVector2::get_y);
	classBoxedVector2->addProperty< float >("length", 0, &BoxedVector2::get_length);
	classBoxedVector2->addMethod("set", &BoxedVector2::set);
	classBoxedVector2->addMethod("dot", &BoxedVector2::dot);
	classBoxedVector2->addMethod("normalized", &BoxedVector2::normalized);
	classBoxedVector2->addMethod("neg", &BoxedVector2::neg);
	classBoxedVector2->addMethod("perpendicular", &BoxedVector2::perpendicular);
	classBoxedVector2->addStaticMethod("lerp", &BoxedVector2::lerp);
	classBoxedVector2->addStaticMethod("distance", &BoxedVector2::distance);
	classBoxedVector2->addOperator< Vector2, const BoxedVector2* >('+', &BoxedVector2::add);
	classBoxedVector2->addOperator< Vector2, float >('+', &BoxedVector2::add);
	classBoxedVector2->addOperator< Vector2, const BoxedVector2* >('-', &BoxedVector2::sub);
	classBoxedVector2->addOperator< Vector2, float >('-', &BoxedVector2::sub);
	classBoxedVector2->addOperator< Vector2, const BoxedVector2* >('*', &BoxedVector2::mul);
	classBoxedVector2->addOperator< Vector2, float >('*', &BoxedVector2::mul);
	classBoxedVector2->addOperator< Vector2, const BoxedVector2* >('/', &BoxedVector2::div);
	classBoxedVector2->addOperator< Vector2, float >('/', &BoxedVector2::div);
	registrar->registerClass(classBoxedVector2);

	auto classBoxedVector4 = new AutoRuntimeClass< BoxedVector4 >();
	classBoxedVector4->addConstructor();
	classBoxedVector4->addConstructor< float, float, float >();
	classBoxedVector4->addConstructor< float, float, float, float >();
	classBoxedVector4->addConstant("zero", CastAny< Vector4 >::set(Vector4::zero()));
	classBoxedVector4->addConstant("origo", CastAny< Vector4 >::set(Vector4::origo()));
	classBoxedVector4->addProperty< float >("x", &BoxedVector4::set_x, &BoxedVector4::get_x);
	classBoxedVector4->addProperty< float >("y", &BoxedVector4::set_y, &BoxedVector4::get_y);
	classBoxedVector4->addProperty< float >("z", &BoxedVector4::set_z, &BoxedVector4::get_z);
	classBoxedVector4->addProperty< float >("w", &BoxedVector4::set_w, &BoxedVector4::get_w);
	classBoxedVector4->addProperty< Vector4 >("xyz0", &BoxedVector4::get_xyz0);
	classBoxedVector4->addProperty< Vector4 >("xyz1", &BoxedVector4::get_xyz1);
	classBoxedVector4->addProperty< float >("length", &BoxedVector4::get_length);
	classBoxedVector4->addMethod("set", &BoxedVector4::set);
	classBoxedVector4->addMethod("set", &BoxedVector4::set_xyzw);
	classBoxedVector4->addMethod("dot", &BoxedVector4::dot);
	classBoxedVector4->addMethod("cross", &BoxedVector4::cross);
	classBoxedVector4->addMethod("normalized", &BoxedVector4::normalized);
	classBoxedVector4->addMethod("neg", &BoxedVector4::neg);
	classBoxedVector4->addStaticMethod("lerp", &BoxedVector4::lerp);
	classBoxedVector4->addStaticMethod("distance3", &BoxedVector4::distance3);
	classBoxedVector4->addStaticMethod("distance4", &BoxedVector4::distance4);
	classBoxedVector4->addStaticMethod("minorAxis3", &BoxedVector4::minorAxis3);
	classBoxedVector4->addStaticMethod("majorAxis3", &BoxedVector4::majorAxis3);
	classBoxedVector4->addOperator< Vector4, const BoxedVector4* >('+', &BoxedVector4::add);
	classBoxedVector4->addOperator< Vector4, float >('+', &BoxedVector4::add);
	classBoxedVector4->addOperator< Vector4, const BoxedVector4* >('-', &BoxedVector4::sub);
	classBoxedVector4->addOperator< Vector4, float >('-', &BoxedVector4::sub);
	classBoxedVector4->addOperator< Vector4, const BoxedVector4* >('*', &BoxedVector4::mul);
	classBoxedVector4->addOperator< Vector4, float >('*', &BoxedVector4::mul);
	classBoxedVector4->addOperator< Vector4, const BoxedVector4* >('/', &BoxedVector4::div);
	classBoxedVector4->addOperator< Vector4, float >('/', &BoxedVector4::div);
	registrar->registerClass(classBoxedVector4);

	auto classBoxedVector4Array = new AutoRuntimeClass< BoxedVector4Array >();
	classBoxedVector4Array->addConstructor();
	classBoxedVector4Array->addConstructor< uint32_t >();
	classBoxedVector4Array->addProperty("size", &BoxedVector4Array::size);
	classBoxedVector4Array->addMethod("reserve", &BoxedVector4Array::reserve);
	classBoxedVector4Array->addMethod("resize", &BoxedVector4Array::resize);
	classBoxedVector4Array->addMethod("clear", &BoxedVector4Array::clear);
	classBoxedVector4Array->addMethod("push_back", &BoxedVector4Array::push_back);
	classBoxedVector4Array->addMethod("pop_back", &BoxedVector4Array::pop_back);
	classBoxedVector4Array->addMethod("front", &BoxedVector4Array::front);
	classBoxedVector4Array->addMethod("back", &BoxedVector4Array::back);
	classBoxedVector4Array->addMethod("set", &BoxedVector4Array::set);
	classBoxedVector4Array->addMethod("get", &BoxedVector4Array::get);
	registrar->registerClass(classBoxedVector4Array);

	auto classBoxedWinding2 = new AutoRuntimeClass< BoxedWinding2 >();
	classBoxedWinding2->addConstructor();
	classBoxedWinding2->addProperty("size", &BoxedWinding2::size);
	classBoxedWinding2->addProperty("empty", &BoxedWinding2::empty);
	classBoxedWinding2->addMethod("clear", &BoxedWinding2::clear);
	classBoxedWinding2->addMethod("push", &BoxedWinding2::push);
	classBoxedWinding2->addMethod("inside", &BoxedWinding2::inside);
	classBoxedWinding2->addMethod("closest", &BoxedWinding2::closest);
	classBoxedWinding2->addMethod("reserve", &BoxedWinding2::reserve);
	classBoxedWinding2->addMethod("resize", &BoxedWinding2::resize);
	classBoxedWinding2->addMethod("get", &BoxedWinding2::get);
	classBoxedWinding2->addMethod("set", &BoxedWinding2::set);
	registrar->registerClass(classBoxedWinding2);

	auto classBoxedWinding3 = new AutoRuntimeClass< BoxedWinding3 >();
	classBoxedWinding3->addConstructor();
	classBoxedWinding3->addProperty("size", &BoxedWinding3::size);
	classBoxedWinding3->addProperty("empty", &BoxedWinding3::empty);
	classBoxedWinding3->addMethod("clear", &BoxedWinding3::clear);
	classBoxedWinding3->addMethod("push", &BoxedWinding3::push);
	classBoxedWinding3->addMethod("reserve", &BoxedWinding3::reserve);
	classBoxedWinding3->addMethod("resize", &BoxedWinding3::resize);
	classBoxedWinding3->addMethod("get", &BoxedWinding3::get);
	classBoxedWinding3->addMethod("set", &BoxedWinding3::set);
	registrar->registerClass(classBoxedWinding3);
}

}
