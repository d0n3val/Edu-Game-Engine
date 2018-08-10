/* Copyright 2011 Jukka Jyl�nki

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

/** @file MathBuildConfig.h
	@author Jukka Jyl�nki
	@brief Specifies all build flags for the library. */
#pragma once

#if !defined(WIN32)
#define WIN32
#endif

// Ric
// Warning disabled ---
#pragma warning( disable : 4577 ) // Warning that exceptions are disabled
#pragma warning( disable : 4530 ) // Warning that exceptions are disabled
#define MATH_SILENT_ASSUME
#define MATH_BULLET_INTEROP
// ---

// Disable STL exceptions
#if !defined(_HAS_EXCEPTIONS) 
#define _HAS_EXCEPTIONS 0
#endif

#define _STATIC_CPPLIB

// If MATH_ENABLE_NAMESPACE is defined, all math symbols are put inside a namespace.
#define MATH_ENABLE_NAMESPACE

// Adjust this #define to choose the name of the namespace math is given to.
// This flag does not have any effect if MATH_ENABLE_NAMESPACE is not defined.
#define MATH_NAMESPACE_NAME math

// If MATH_AUTO_USE_NAMESPACE is defined, a 'using namespace math;' directive is issued in all header files, causing
// the math namespace to exist only for internal symbol hiding purposes, and not for external code.
// This flag does not have any effect if MATH_ENABLE_NAMESPACE is not defined.
#define MATH_AUTO_USE_NAMESPACE

// Detect if we're building on Windows 8 and mark it in define WIN8 if so.
#if defined(WINVER) && !defined(WIN8)
#if WINVER >= 0x0602
#define WIN8
#endif
#endif

// If MATH_ENABLE_WINXP_SUPPORT is defined, we avoid using Windows symbols that require Vista or newer (e.g. GetTickCount64)
#if !defined(WIN8) /* Win8 cannot build with XP support - GetTickCount() doesn't exist. */
#define MATH_ENABLE_WINXP_SUPPORT
#endif

// If MATH_ASSERT_ON_ASSUME is defined, assume() resolves directly to assert().
// When not defined, assume() prints out an error if the condition fails, but continues
// execution.
//#define MATH_ASSERT_ON_ASSUME

#ifndef _DEBUG
// If MATH_SILENT_ASSUME is defined, all assume() tests are stripped from the build. This
// overrides MATH_ASSERT_ON_ASSUME.
#ifndef MATH_SILENT_ASSUME
#define MATH_SILENT_ASSUME
#endif

// If MATH_ENABLE_INSECURE_OPTIMIZATIONS, several security checks (unsafe index out of bounds accesses etc.) are disabled.
// Use this for absolutely fastest performance on MathGeoLib, but only if you know your code is 100% well-formed and uses
// MathGeoLib bug-free.
// #define MATH_ENABLE_INSECURE_OPTIMIZATIONS

#endif

#ifdef _DEBUG
// If MATH_ASSERT_CORRECTNESS is defined, special (and possibly rather costly) mathassert()
// tests are enabled, which test the internal correctness of the library.
//#define MATH_ASSERT_CORRECTNESS
#endif

// If FAIL_USING_EXCEPTIONS is defined, all mathassert(), assert() and assume() macros turn into
// throwing std::runtime_error exceptions. This macro is used by default when running tests, to get
// a runtime error report on which tests pass and which fail.
#ifndef FAIL_USING_EXCEPTIONS
// #define FAIL_USING_EXCEPTIONS
#endif

// If MATH_ENABLE_STL_SUPPORT is defined, MathGeoLib utilizes STL data structures. Otherwise,
// features requiring STL are disabled (but the library can still be built).
#ifndef MATH_ENABLE_STL_SUPPORT
	#define MATH_ENABLE_STL_SUPPORT
#endif

// If MATH_TINYXML_INTEROP is defined, MathGeoLib integrates with TinyXML to provide
// serialization and deserialization to XML for the data structures.
#ifndef MATH_TINYXML_INTEROP
//#define MATH_TINYXML_INTEROP
#endif

// If MATH_CONTAINERLIB_SUPPORT is defined, MathGeoLib integrates with a certain
// STL replacement container library. Do not enable, only for internal use.
#ifndef MATH_CONTAINERLIB_SUPPORT
//#define MATH_CONTAINERLIB_SUPPORT
#endif

// If MATH_GRAPHICSENGINE_INTEROP is defined, MathGeoLib integrates with a certain
// graphics engine. Do not enable, only for internal use.
#ifndef MATH_GRAPHICSENGINE_INTEROP
//#define MATH_GRAPHICSENGINE_INTEROP
#endif

// If KNET_LOGGING_SUPPORT_ENABLED is defined, MathGeoLib utilizes logging functions
// from the kNet library.
#ifndef KNET_LOGGING_SUPPORT_ENABLED
//#define KNET_LOGGING_SUPPORT_ENABLED
#endif

// Uncomment to specify the SIMD instruction set level in use.
//#define MATH_AVX
//#define MATH_SSE41
//#define MATH_SSE3
//#define MATH_SSE2
//#define MATH_SSE // SSE1.

///\todo Test iOS support.
///\todo Enable NEON only on ARMv7, not older.
//#if (defined(ANDROID) && defined(__ARM_ARCH_7A__)) || (defined(WIN8RT) && defined(_M_ARM))
//#define MATH_NEON
#ifdef MATH_NEON
#include <arm_neon.h>
#endif

// MATH_AVX implies MATH_SSE41, which implies MATH_SSE3, which implies MATH_SSE2, which implies MATH_SSE.
#ifdef MATH_AVX
#if defined(__GNUC__) || defined(__clang__)
#include <immintrin.h>
#endif
#ifndef MATH_SSE41
#define MATH_SSE41
#endif
#define MATH_SSE3
#define MATH_SSE2
#define MATH_SSE
#endif

#ifdef MATH_SSE41
#define MATH_SSE3
#define MATH_SSE2
#define MATH_SSE
#endif

#ifdef MATH_SSE3
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <pmmintrin.h>
#endif
#define MATH_SSE2
#define MATH_SSE
#endif

#ifdef MATH_SSE2
#define MATH_SSE
#include <emmintrin.h>
#endif

#ifdef MATH_SSE
#include <xmmintrin.h>
#endif

#if defined(MATH_SSE) || defined(MATH_NEON)
#define MATH_SIMD // A common #define to signal the simd4f type is available.

#ifdef MATH_NEON
typedef float32x4_t simd4f;
#elif defined(MATH_SSE)
typedef __m128 simd4f;
#endif

#endif

#if defined(MATH_SIMD) && !defined(MATH_AUTOMATIC_SSE)
// Automatically use the SSE-optimized operations for all code.
// This should only be disabled for benchmarking purposes.
#define MATH_AUTOMATIC_SSE
#endif

#include "Math/MathTypes.h"
