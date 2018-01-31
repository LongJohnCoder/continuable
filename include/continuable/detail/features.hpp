/*

                        /~` _  _ _|_. _     _ |_ | _
                        \_,(_)| | | || ||_|(_||_)|(/_

                    https://github.com/Naios/continuable
                                   v2.0.0

  Copyright(c) 2015 - 2018 Denis Blank <denis.blank at outlook dot com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files(the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions :

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
**/

#ifndef CONTINUABLE_DETAIL_FEATURES_HPP_INCLUDED_
#define CONTINUABLE_DETAIL_FEATURES_HPP_INCLUDED_

// Defines CONTINUABLE_WITH_NO_EXCEPTIONS when exception support is disabled
#ifndef CONTINUABLE_WITH_NO_EXCEPTIONS
#if defined(_MSC_VER)
#if !defined(_HAS_EXCEPTIONS) || (_HAS_EXCEPTIONS == 0)
#define CONTINUABLE_WITH_NO_EXCEPTIONS
#endif
#elif defined(__clang__)
#if !(__EXCEPTIONS && __has_feature(cxx_exceptions))
#define CONTINUABLE_WITH_NO_EXCEPTIONS
#endif
#elif defined(__GNUC__)
#if !__EXCEPTIONS
#define CONTINUABLE_WITH_NO_EXCEPTIONS
#endif
#endif
#endif // CONTINUABLE_WITH_NO_EXCEPTIONS

/// Define CONTINUABLE_WITH_EXCEPTIONS when exceptions are used
#if !defined(CONTINUABLE_WITH_CUSTOM_ERROR_TYPE) &&                            \
    !defined(CONTINUABLE_WITH_NO_EXCEPTIONS)
#define CONTINUABLE_WITH_EXCEPTIONS 1
#else
#undef CONTINUABLE_WITH_EXCEPTIONS
#endif

#if defined(__has_feature)
#if __has_feature(cxx_if_constexpr)
// PR not merged into the clang master yet
#define CONTINUABLE_HAS_CXX17_CONSTEXPR_IF
#endif

#if __has_feature(cxx_fold_expressions)
// PR not merged into the clang master yet
#define CONTINUABLE_HAS_CXX17_FOLD_EXPRESSION
#endif
#endif

/// This is enabled by the CMake project
// #undef CONTINUABLE_HAS_EXPERIMENTAL_COROUTINE

#endif // CONTINUABLE_DETAIL_FEATURES_HPP_INCLUDED_
