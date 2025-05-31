/* paul.h -- https://github.com/takeiteasy/paul

paul Copyright (C) 2025 George Watson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef PAUL_HEAD
#define PAUL_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define COMPILER_CL

#ifdef _WIN32
#define PLATFORM_WINDOWS
#else
#error compiler/platform combo is not supported yet
#endif

#if defined(_M_AMD64)
#define ARCH_X64
#elif defined(_M_IX86)
#define ARCH_X86
#elif defined(_M_ARM64)
#define ARCH_ARM64
#elif defined(_M_ARM)
#define ARCH_ARM32
#else
#error architecture not supported yet
#endif

#if _MSC_VER >= 1920
#define COMPILER_CL_YEAR 2019
#elif _MSC_VER >= 1910
#define COMPILER_CL_YEAR 2017
#elif _MSC_VER >= 1900
#define COMPILER_CL_YEAR 2015
#elif _MSC_VER >= 1800
#define COMPILER_CL_YEAR 2013
#elif _MSC_VER >= 1700
#define COMPILER_CL_YEAR 2012
#elif _MSC_VER >= 1600
#define COMPILER_CL_YEAR 2010
#elif _MSC_VER >= 1500
#define COMPILER_CL_YEAR 2008
#elif _MSC_VER >= 1400
#define COMPILER_CL_YEAR 2005
#else
#define COMPILER_CL_YEAR 0
#endif
// defined(_MSC_VER)
#elif defined(__clang__)
#define COMPILER_CLANG

#if defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_MAC
#elif defined(__gnu_linux__)
#define PLATFORM_LINUX
#else
#error compiler/platform combo is not supported yet
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#define ARCH_X64
#elif defined(i386) || defined(__i386) || defined(__i386__)
#define ARCH_X86
#elif defined(__aarch64__)
#define ARCH_ARM64
#elif defined(__arm__)
#define ARCH_ARM32
#else
#error architecture not supported yet
#endif
// defined(__clang__)
#elif defined(__GNUC__) || defined(__GNUG__)
#define COMPILER_GCC

#ifdef __gnu_linux__
#define PLATFORM_LINUX
#else
#error compiler/platform combo is not supported yet
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#define ARCH_X64
#elif defined(i386) || defined(__i386) || defined(__i386__)
#define ARCH_X86
#elif defined(__aarch64__)
#define ARCH_ARM64
#elif defined(__arm__)
#define ARCH_ARM32
#else
#error architecture not supported yet
#endif
#else
#error compiler is not supported yet
#endif

#if defined(ARCH_X64)
#define ARCH_64BIT
#elif defined(ARCH_X86)
#define ARCH_32BIT
// defined(__GNUC__) || defined(__GNUP__)
#endif

#ifdef __cplusplus
#define LANG_CPP
#else
#define LANG_C
#endif

#ifndef PLATFORM_WINDOWS
#define PLATFORM_POSIX
#endif

#ifdef __cplusplus
}
#endif
#endif // PAUL_HEAD
