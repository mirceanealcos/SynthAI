/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce::dsp
{

namespace SIMDRegister_test_internal
{
    template <typename type>
    struct RandomPrimitive
    {
        static type next (Random& random)
        {
            if constexpr (std::is_floating_point_v<type>)
            {
                return static_cast<type> (std::is_signed_v<type> ? (random.nextFloat() * 16.0) - 8.0
                                                                 : (random.nextFloat() * 8.0));
            }
            else if constexpr (std::is_integral_v<type>)
            {
                return static_cast<type> (random.nextInt64());
            }
        }
    };

    template <typename type>
    struct RandomValue
    {
        static type next (Random& random)
        {
            return RandomPrimitive<type>::next (random);
        }
    };

    template <typename type>
    struct RandomValue<std::complex<type>>
    {
        static std::complex<type> next (Random& random)
        {
            return {RandomPrimitive<type>::next (random), RandomPrimitive<type>::next (random)};
        }
    };

    template <typename type>
    static void fillVec (type* dst, Random& random)
    {
        std::generate_n (dst, SIMDRegister<type>::SIMDNumElements, [&]
        {
            return RandomValue<type>::next (random);
        });
    }

    // Avoid visual studio warning
    template <typename type>
    static type safeAbs (type a)
    {
        return static_cast<type> (std::abs (static_cast<double> (a)));
    }

    template <typename type>
    static type safeAbs (std::complex<type> a)
    {
        return std::abs (a);
    }

    template <typename type>
    static double difference (type a)
    {
        return static_cast<double> (safeAbs (a));
    }

    template <typename type>
    static double difference (type a, type b)
    {
        return difference (a - b);
    }
} // namespace SIMDRegister_test_internal

// These tests need to be strictly run on all platforms supported by JUCE as the
// SIMD code is highly platform dependent.

class SIMDRegisterUnitTests final : public UnitTest
{
public:
    template <typename> struct Tag {};

    SIMDRegisterUnitTests()
        : UnitTest ("SIMDRegister UnitTests", UnitTestCategories::dsp)
    {}

    //==============================================================================
    // Some helper classes
    template <typename type>
    static bool allValuesEqualTo (const SIMDRegister<type>& vec, const type scalar)
    {
        alignas (sizeof (SIMDRegister<type>)) type elements[SIMDRegister<type>::SIMDNumElements];

        vec.copyToRawArray (elements);

        // as we do not want to rely on the access operator we cast this to a primitive pointer
        return std::all_of (std::begin (elements), std::end (elements), [scalar] (const auto x) { return exactlyEqual (x, scalar); });
    }

    template <typename type>
    static bool vecEqualToArray (const SIMDRegister<type>& vec, const type* array)
    {
        HeapBlock<type> vecElementsStorage (SIMDRegister<type>::SIMDNumElements * 2);
        auto* ptr = SIMDRegister<type>::getNextSIMDAlignedPtr (vecElementsStorage.getData());
        vec.copyToRawArray (ptr);

        for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
        {
            double delta = SIMDRegister_test_internal::difference (ptr[i], array[i]);
            if (delta > 1e-4)
            {
                DBG ("a: " << SIMDRegister_test_internal::difference (ptr[i]) << " b: " << SIMDRegister_test_internal::difference (array[i]) << " difference: " << delta);
                return false;
            }
        }

        return true;
    }

    template <typename type>
    static void copy (SIMDRegister<type>& vec, const type* ptr)
    {
        if (SIMDRegister<type>::isSIMDAligned (ptr))
        {
            vec = SIMDRegister<type>::fromRawArray (ptr);
        }
        else
        {
            for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
                vec[i] = ptr[i];
        }
    }

    //==============================================================================
    // Some useful operations to test
    struct Addition
    {
        template <typename typeOne, typename typeTwo>
        static void inplace (typeOne& a, const typeTwo& b)
        {
            a += b;
        }

        template <typename typeOne, typename typeTwo>
        static typeOne outofplace (const typeOne& a, const typeTwo& b)
        {
            return a + b;
        }
    };

    struct Subtraction
    {
        template <typename typeOne, typename typeTwo>
        static void inplace (typeOne& a, const typeTwo& b)
        {
            a -= b;
        }

        template <typename typeOne, typename typeTwo>
        static typeOne outofplace (const typeOne& a, const typeTwo& b)
        {
            return a - b;
        }
    };

    struct Multiplication
    {
        template <typename typeOne, typename typeTwo>
        static void inplace (typeOne& a, const typeTwo& b)
        {
            a *= b;
        }

        template <typename typeOne, typename typeTwo>
        static typeOne outofplace (const typeOne& a, const typeTwo& b)
        {
            return a * b;
        }
    };

    struct BitAND
    {
        template <typename typeOne, typename typeTwo>
        static void inplace (typeOne& a, const typeTwo& b)
        {
            a &= b;
        }

        template <typename typeOne, typename typeTwo>
        static typeOne outofplace (const typeOne& a, const typeTwo& b)
        {
            return a & b;
        }
    };

    struct BitOR
    {
        template <typename typeOne, typename typeTwo>
        static void inplace (typeOne& a, const typeTwo& b)
        {
            a |= b;
        }

        template <typename typeOne, typename typeTwo>
        static typeOne outofplace (const typeOne& a, const typeTwo& b)
        {
            return a | b;
        }
    };

    struct BitXOR
    {
        template <typename typeOne, typename typeTwo>
        static void inplace (typeOne& a, const typeTwo& b)
        {
            a ^= b;
        }

        template <typename typeOne, typename typeTwo>
        static typeOne outofplace (const typeOne& a, const typeTwo& b)
        {
            return a ^ b;
        }
    };

    //==============================================================================
    // the individual tests
    struct InitializationTest
    {
        template <typename type>
        static void run (UnitTest& u, Random& random, Tag<type>)
        {
            u.expect (allValuesEqualTo<type> (SIMDRegister<type>::expand (static_cast<type> (23)), 23));

            {
               #ifdef _MSC_VER
                __declspec (align (sizeof (SIMDRegister<type>))) type elements[SIMDRegister<type>::SIMDNumElements];
               #else
                type elements[SIMDRegister<type>::SIMDNumElements] __attribute__ ((aligned (sizeof (SIMDRegister<type>))));
               #endif
                SIMDRegister_test_internal::fillVec (elements, random);
                SIMDRegister<type> a (SIMDRegister<type>::fromRawArray (elements));

                u.expect (vecEqualToArray (a, elements));

                SIMDRegister<type> b (a);
                a *= static_cast<type> (2);

                u.expect (vecEqualToArray (b, elements));
            }
        }
    };

    struct AccessTest
    {
        template <typename type>
        static void run (UnitTest& u, Random& random, Tag<type>)
        {
            // set-up
            SIMDRegister<type> a;
            type array [SIMDRegister<type>::SIMDNumElements];

            SIMDRegister_test_internal::fillVec (array, random);

            // Test non-const access operator
            for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
                a[i] = array[i];

            u.expect (vecEqualToArray (a, array));

            // Test const access operator
            const SIMDRegister<type>& b = a;

            for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
                u.expect (exactlyEqual (b[i], array[i]));
        }
    };

    template <class Operation>
    struct OperatorTests
    {
        template <typename type>
        static void run (UnitTest& u, Random& random, Tag<type>)
        {
            for (int n = 0; n < 100; ++n)
            {
                // set-up
                SIMDRegister<type> a (static_cast<type> (0));
                SIMDRegister<type> b (static_cast<type> (0));
                SIMDRegister<type> c (static_cast<type> (0));

                type array_a [SIMDRegister<type>::SIMDNumElements];
                type array_b [SIMDRegister<type>::SIMDNumElements];
                type array_c [SIMDRegister<type>::SIMDNumElements];

                SIMDRegister_test_internal::fillVec (array_a, random);
                SIMDRegister_test_internal::fillVec (array_b, random);
                SIMDRegister_test_internal::fillVec (array_c, random);

                copy (a, array_a); copy (b, array_b); copy (c, array_c);

                // test in-place with both params being vectors
                for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
                    Operation::template inplace<type, type> (array_a[i], array_b[i]);

                Operation::template inplace<SIMDRegister<type>, SIMDRegister<type>> (a, b);

                u.expect (vecEqualToArray (a, array_a));
                u.expect (vecEqualToArray (b, array_b));

                SIMDRegister_test_internal::fillVec (array_a, random);
                SIMDRegister_test_internal::fillVec (array_b, random);
                SIMDRegister_test_internal::fillVec (array_c, random);

                copy (a, array_a); copy (b, array_b); copy (c, array_c);

                // test in-place with one param being scalar
                for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
                    Operation::template inplace<type, type> (array_b[i], static_cast<type> (2));

                Operation::template inplace<SIMDRegister<type>, type> (b, 2);

                u.expect (vecEqualToArray (a, array_a));
                u.expect (vecEqualToArray (b, array_b));

                // set-up again
                SIMDRegister_test_internal::fillVec (array_a, random);
                SIMDRegister_test_internal::fillVec (array_b, random);
                SIMDRegister_test_internal::fillVec (array_c, random);
                copy (a, array_a); copy (b, array_b); copy (c, array_c);

                // test out-of-place with both params being vectors
                for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
                    array_c[i] = Operation::template outofplace<type, type> (array_a[i], array_b[i]);

                c = Operation::template outofplace<SIMDRegister<type>, SIMDRegister<type>> (a, b);

                u.expect (vecEqualToArray (a, array_a));
                u.expect (vecEqualToArray (b, array_b));
                u.expect (vecEqualToArray (c, array_c));

                // test out-of-place with one param being scalar
                for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
                    array_c[i] = Operation::template outofplace<type, type> (array_b[i], static_cast<type> (2));

                c = Operation::template outofplace<SIMDRegister<type>, type> (b, 2);

                u.expect (vecEqualToArray (a, array_a));
                u.expect (vecEqualToArray (b, array_b));
                u.expect (vecEqualToArray (c, array_c));
            }
        }
    };

    template <class Operation>
    struct BitOperatorTests
    {
        template <typename type>
        static void run (UnitTest& u, Random& random, Tag<type>)
        {
            typedef typename SIMDRegister<type>::vMaskType vMaskType;
            typedef typename SIMDRegister<type>::MaskType MaskType;

            for (int n = 0; n < 100; ++n)
            {
                // Check flip sign bit and using as a union
                {
                    type array_a [SIMDRegister<type>::SIMDNumElements];

                    union ConversionUnion
                    {
                        inline ConversionUnion() : floatVersion (static_cast<type> (0)) {}
                        inline ~ConversionUnion() {}
                        SIMDRegister<type> floatVersion;
                        vMaskType intVersion;
                    } a, b;

                    vMaskType bitmask = vMaskType::expand (static_cast<MaskType> (1) << (sizeof (MaskType) - 1));
                    SIMDRegister_test_internal::fillVec (array_a, random);
                    copy (a.floatVersion, array_a);
                    copy (b.floatVersion, array_a);

                    Operation::template inplace<SIMDRegister<type>, vMaskType> (a.floatVersion, bitmask);
                    Operation::template inplace<vMaskType, vMaskType> (b.intVersion, bitmask);

                   #ifdef _MSC_VER
                    __declspec (align (sizeof (SIMDRegister<type>))) type elements[SIMDRegister<type>::SIMDNumElements];
                   #else
                    type elements[SIMDRegister<type>::SIMDNumElements] __attribute__ ((aligned (sizeof (SIMDRegister<type>))));
                   #endif
                    b.floatVersion.copyToRawArray (elements);

                    u.expect (vecEqualToArray (a.floatVersion, elements));
                }

                // set-up
                SIMDRegister<type> a, c;
                vMaskType b;

                MaskType array_a [SIMDRegister<MaskType>::SIMDNumElements];
                MaskType array_b [SIMDRegister<MaskType>::SIMDNumElements];
                MaskType array_c [SIMDRegister<MaskType>::SIMDNumElements];

                type float_a [SIMDRegister<type>::SIMDNumElements];
                type float_c [SIMDRegister<type>::SIMDNumElements];

                SIMDRegister_test_internal::fillVec (float_a, random);
                SIMDRegister_test_internal::fillVec (array_b, random);
                SIMDRegister_test_internal::fillVec (float_c, random);

                memcpy (array_a, float_a, sizeof (type) * SIMDRegister<type>::SIMDNumElements);
                memcpy (array_c, float_c, sizeof (type) * SIMDRegister<type>::SIMDNumElements);
                copy (a, float_a); copy (b, array_b); copy (c, float_c);

                // test in-place with both params being vectors
                for (size_t i = 0; i < SIMDRegister<MaskType>::SIMDNumElements; ++i)
                    Operation::template inplace<MaskType, MaskType> (array_a[i], array_b[i]);
                memcpy (float_a, array_a, sizeof (type) * SIMDRegister<type>::SIMDNumElements);

                Operation::template inplace<SIMDRegister<type>, vMaskType> (a, b);

                u.expect (vecEqualToArray (a, float_a));
                u.expect (vecEqualToArray (b, array_b));

                SIMDRegister_test_internal::fillVec (float_a, random);
                SIMDRegister_test_internal::fillVec (array_b, random);
                SIMDRegister_test_internal::fillVec (float_c, random);
                memcpy (array_a, float_a, sizeof (type) * SIMDRegister<type>::SIMDNumElements);
                memcpy (array_c, float_c, sizeof (type) * SIMDRegister<type>::SIMDNumElements);
                copy (a, float_a); copy (b, array_b); copy (c, float_c);

                // test in-place with one param being scalar
                for (size_t i = 0; i < SIMDRegister<MaskType>::SIMDNumElements; ++i)
                    Operation::template inplace<MaskType, MaskType> (array_a[i], static_cast<MaskType> (9));
                memcpy (float_a, array_a, sizeof (type) * SIMDRegister<type>::SIMDNumElements);

                Operation::template inplace<SIMDRegister<type>, MaskType> (a, static_cast<MaskType> (9));

                u.expect (vecEqualToArray (a, float_a));
                u.expect (vecEqualToArray (b, array_b));

                // set-up again
                SIMDRegister_test_internal::fillVec (float_a, random);
                SIMDRegister_test_internal::fillVec (array_b, random);
                SIMDRegister_test_internal::fillVec (float_c, random);
                memcpy (array_a, float_a, sizeof (type) * SIMDRegister<type>::SIMDNumElements);
                memcpy (array_c, float_c, sizeof (type) * SIMDRegister<type>::SIMDNumElements);
                copy (a, float_a); copy (b, array_b); copy (c, float_c);

                // test out-of-place with both params being vectors
                for (size_t i = 0; i < SIMDRegister<MaskType>::SIMDNumElements; ++i)
                {
                    array_c[i] =
                        Operation::template outofplace<MaskType, MaskType> (array_a[i], array_b[i]);
                }
                memcpy (float_a, array_a, sizeof (type) * SIMDRegister<type>::SIMDNumElements);
                memcpy (float_c, array_c, sizeof (type) * SIMDRegister<type>::SIMDNumElements);

                c = Operation::template outofplace<SIMDRegister<type>, vMaskType> (a, b);

                u.expect (vecEqualToArray (a, float_a));
                u.expect (vecEqualToArray (b, array_b));
                u.expect (vecEqualToArray (c, float_c));

                // test out-of-place with one param being scalar
                for (size_t i = 0; i < SIMDRegister<MaskType>::SIMDNumElements; ++i)
                    array_c[i] = Operation::template outofplace<MaskType, MaskType> (array_a[i], static_cast<MaskType> (9));
                memcpy (float_a, array_a, sizeof (type) * SIMDRegister<type>::SIMDNumElements);
                memcpy (float_c, array_c, sizeof (type) * SIMDRegister<type>::SIMDNumElements);

                c = Operation::template outofplace<SIMDRegister<type>, MaskType> (a, static_cast<MaskType> (9));

                u.expect (vecEqualToArray (a, float_a));
                u.expect (vecEqualToArray (b, array_b));
                u.expect (vecEqualToArray (c, float_c));
            }
        }
    };

    struct CheckComparisonOps
    {
        template <typename type>
        static void run (UnitTest& u, Random& random, Tag<type>)
        {
            typedef typename SIMDRegister<type>::vMaskType vMaskType;
            typedef typename SIMDRegister<type>::MaskType MaskType;

            for (int i = 0; i < 100; ++i)
            {
                // set-up
                type array_a   [SIMDRegister<type>::SIMDNumElements];
                type array_b   [SIMDRegister<type>::SIMDNumElements];
                MaskType array_eq  [SIMDRegister<type>::SIMDNumElements];
                MaskType array_neq [SIMDRegister<type>::SIMDNumElements];
                MaskType array_lt  [SIMDRegister<type>::SIMDNumElements];
                MaskType array_le  [SIMDRegister<type>::SIMDNumElements];
                MaskType array_gt  [SIMDRegister<type>::SIMDNumElements];
                MaskType array_ge  [SIMDRegister<type>::SIMDNumElements];


                SIMDRegister_test_internal::fillVec (array_a, random);
                SIMDRegister_test_internal::fillVec (array_b, random);

                // do check
                for (size_t j = 0; j < SIMDRegister<type>::SIMDNumElements; ++j)
                {
                    array_eq  [j] = (  exactlyEqual (array_a[j], array_b[j])) ? static_cast<MaskType> (-1) : 0;
                    array_neq [j] = (! exactlyEqual (array_a[j], array_b[j])) ? static_cast<MaskType> (-1) : 0;
                    array_lt  [j] = (array_a[j] <  array_b[j]) ? static_cast<MaskType> (-1) : 0;
                    array_le  [j] = (array_a[j] <= array_b[j]) ? static_cast<MaskType> (-1) : 0;
                    array_gt  [j] = (array_a[j] >  array_b[j]) ? static_cast<MaskType> (-1) : 0;
                    array_ge  [j] = (array_a[j] >= array_b[j]) ? static_cast<MaskType> (-1) : 0;
                }

                SIMDRegister<type> a (static_cast<type> (0));
                SIMDRegister<type> b (static_cast<type> (0));

                vMaskType eq, neq, lt, le, gt, ge;

                copy (a, array_a);
                copy (b, array_b);

                eq  = SIMDRegister<type>::equal              (a, b);
                neq = SIMDRegister<type>::notEqual           (a, b);
                lt  = SIMDRegister<type>::lessThan           (a, b);
                le  = SIMDRegister<type>::lessThanOrEqual    (a, b);
                gt  = SIMDRegister<type>::greaterThan        (a, b);
                ge  = SIMDRegister<type>::greaterThanOrEqual (a, b);

                u.expect (vecEqualToArray (eq,  array_eq ));
                u.expect (vecEqualToArray (neq, array_neq));
                u.expect (vecEqualToArray (lt,  array_lt ));
                u.expect (vecEqualToArray (le,  array_le ));
                u.expect (vecEqualToArray (gt,  array_gt ));
                u.expect (vecEqualToArray (ge,  array_ge ));

                do
                {
                    SIMDRegister_test_internal::fillVec (array_a, random);
                    SIMDRegister_test_internal::fillVec (array_b, random);
                } while (std::equal (array_a, array_a + SIMDRegister<type>::SIMDNumElements, array_b));

                copy (a, array_a);
                copy (b, array_b);
                u.expect (a != b);
                u.expect (b != a);
                u.expect (! (a == b));
                u.expect (! (b == a));

                SIMDRegister_test_internal::fillVec (array_a, random);
                copy (a, array_a);
                copy (b, array_a);

                u.expect (a == b);
                u.expect (b == a);
                u.expect (! (a != b));
                u.expect (! (b != a));

                type scalar = a[0];
                a = SIMDRegister<type>::expand (scalar);

                u.expect (a == scalar);
                u.expect (! (a != scalar));

                scalar--;

                u.expect (a != scalar);
                u.expect (! (a == scalar));
            }
        }
    };

    struct CheckMultiplyAdd
    {
        template <typename type>
        static void run (UnitTest& u, Random& random, Tag<type>)
        {
            // set-up
            type array_a [SIMDRegister<type>::SIMDNumElements];
            type array_b [SIMDRegister<type>::SIMDNumElements];
            type array_c [SIMDRegister<type>::SIMDNumElements];
            type array_d [SIMDRegister<type>::SIMDNumElements];

            SIMDRegister_test_internal::fillVec (array_a, random);
            SIMDRegister_test_internal::fillVec (array_b, random);
            SIMDRegister_test_internal::fillVec (array_c, random);
            SIMDRegister_test_internal::fillVec (array_d, random);

            // check
            for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
                array_d[i] = array_a[i] + (array_b[i] * array_c[i]);

            SIMDRegister<type> a, b, c, d;

            copy (a, array_a);
            copy (b, array_b);
            copy (c, array_c);

            d = SIMDRegister<type>::multiplyAdd (a, b, c);

            u.expect (vecEqualToArray (d, array_d));
        }
    };

    struct CheckMinMax
    {
        template <typename type>
        static void run (UnitTest& u, Random& random, Tag<type>)
        {
            for (int i = 0; i < 100; ++i)
            {
                type array_a [SIMDRegister<type>::SIMDNumElements];
                type array_b [SIMDRegister<type>::SIMDNumElements];
                type array_min [SIMDRegister<type>::SIMDNumElements];
                type array_max [SIMDRegister<type>::SIMDNumElements];

                for (size_t j = 0; j < SIMDRegister<type>::SIMDNumElements; ++j)
                {
                    array_a[j] = static_cast<type> (random.nextInt (127));
                    array_b[j] = static_cast<type> (random.nextInt (127));
                }

                for (size_t j = 0; j < SIMDRegister<type>::SIMDNumElements; ++j)
                {
                    array_min[j] = (array_a[j] < array_b[j]) ? array_a[j] : array_b[j];
                    array_max[j] = (array_a[j] > array_b[j]) ? array_a[j] : array_b[j];
                }

                SIMDRegister<type> a (static_cast<type> (0));
                SIMDRegister<type> b (static_cast<type> (0));
                SIMDRegister<type> vMin (static_cast<type> (0));
                SIMDRegister<type> vMax (static_cast<type> (0));

                copy (a, array_a);
                copy (b, array_b);

                vMin = jmin (a, b);
                vMax = jmax (a, b);

                u.expect (vecEqualToArray (vMin, array_min));
                u.expect (vecEqualToArray (vMax, array_max));

                copy (vMin, array_a);
                copy (vMax, array_a);

                vMin = SIMDRegister<type>::min (a, b);
                vMax = SIMDRegister<type>::max (a, b);

                u.expect (vecEqualToArray (vMin, array_min));
                u.expect (vecEqualToArray (vMax, array_max));
            }
        }
    };

    struct CheckSum
    {
        template <typename type>
        static void run (UnitTest& u, Random& random, Tag<type>)
        {
            type array [SIMDRegister<type>::SIMDNumElements];

            SIMDRegister_test_internal::fillVec (array, random);

            using AddedType = decltype (type{} + type{});
            const auto sumCheck = (type) std::accumulate (std::begin (array), std::end (array), AddedType{});

            SIMDRegister<type> a;
            copy (a, array);

            u.expect (SIMDRegister_test_internal::difference (sumCheck, a.sum()) < 1e-4);
        }
    };

    struct CheckAbs
    {
        template <typename type>
        static void run (UnitTest& u, Random& random, Tag<type>)
        {
            type inArray[SIMDRegister<type>::SIMDNumElements];
            type outArray[SIMDRegister<type>::SIMDNumElements];

            SIMDRegister_test_internal::fillVec (inArray, random);

            SIMDRegister<type> a;
            copy (a, inArray);
            a = SIMDRegister<type>::abs (a);

            auto calcAbs = [] (type x) -> type { return x >= type (0) ? x : type (-x); };

            for (size_t j = 0; j < SIMDRegister<type>::SIMDNumElements; ++j)
                outArray[j] = calcAbs (inArray[j]);

            u.expect (vecEqualToArray (a, outArray));
        }
    };

    struct CheckTruncate
    {
        template <typename type>
        static void run (UnitTest& u, Random& random, Tag<type>)
        {
            type inArray[SIMDRegister<type>::SIMDNumElements];
            type outArray[SIMDRegister<type>::SIMDNumElements];

            SIMDRegister_test_internal::fillVec (inArray, random);

            SIMDRegister<type> a;
            copy (a, inArray);
            a = SIMDRegister<type>::truncate (a);

            for (size_t j = 0; j < SIMDRegister<type>::SIMDNumElements; ++j)
                outArray[j] = (type) (int) inArray[j];

            u.expect (vecEqualToArray (a, outArray));
        }
    };

    struct CheckBoolEquals
    {
        template <typename type>
        static void run (UnitTest& u, Random& random, Tag<type>)
        {
            type array [SIMDRegister<type>::SIMDNumElements];

            auto value = std::is_signed_v<type> ? static_cast<type> ((random.nextFloat() * 16.0) - 8.0)
                                                : static_cast<type> (random.nextFloat() * 8.0);

            std::fill (array, array + SIMDRegister<type>::SIMDNumElements, value);
            SIMDRegister<type> a, b;
            copy (a, array);

            u.expect (a == value);
            u.expect (! (a != value));
            value += 1;

            u.expect (a != value);
            u.expect (! (a == value));

            SIMDRegister_test_internal::fillVec (array, random);
            copy (a, array);
            copy (b, array);

            u.expect (a == b);
            u.expect (! (a != b));

            SIMDRegister_test_internal::fillVec (array, random);
            copy (b, array);

            u.expect (a != b);
            u.expect (! (a == b));
        }
    };

    //==============================================================================
    template <class TheTest>
    void runTestFloatingPoint (const char* unitTestName, TheTest)
    {
        beginTest (unitTestName);

        Random random = getRandom();

        TheTest::run (*this, random, Tag<float> {});
        TheTest::run (*this, random, Tag<double>{});
    }

    //==============================================================================
    template <class TheTest>
    void runTestForAllTypes (const char* unitTestName, TheTest)
    {
        beginTest (unitTestName);

        Random random = getRandom();

        TheTest::run (*this, random, Tag<float>               {});
        TheTest::run (*this, random, Tag<double>              {});
        TheTest::run (*this, random, Tag<int8_t>              {});
        TheTest::run (*this, random, Tag<uint8_t>             {});
        TheTest::run (*this, random, Tag<int16_t>             {});
        TheTest::run (*this, random, Tag<uint16_t>            {});
        TheTest::run (*this, random, Tag<int32_t>             {});
        TheTest::run (*this, random, Tag<uint32_t>            {});
        TheTest::run (*this, random, Tag<int64_t>             {});
        TheTest::run (*this, random, Tag<uint64_t>            {});
        TheTest::run (*this, random, Tag<std::complex<float>> {});
        TheTest::run (*this, random, Tag<std::complex<double>>{});
    }

    template <class TheTest>
    void runTestNonComplex (const char* unitTestName, TheTest)
    {
        beginTest (unitTestName);

        Random random = getRandom();

        TheTest::run (*this, random, Tag<float>   {});
        TheTest::run (*this, random, Tag<double>  {});
        TheTest::run (*this, random, Tag<int8_t>  {});
        TheTest::run (*this, random, Tag<uint8_t> {});
        TheTest::run (*this, random, Tag<int16_t> {});
        TheTest::run (*this, random, Tag<uint16_t>{});
        TheTest::run (*this, random, Tag<int32_t> {});
        TheTest::run (*this, random, Tag<uint32_t>{});
        TheTest::run (*this, random, Tag<int64_t> {});
        TheTest::run (*this, random, Tag<uint64_t>{});
    }

    template <class TheTest>
    void runTestSigned (const char* unitTestName, TheTest)
    {
        beginTest (unitTestName);

        Random random = getRandom();

        TheTest::run (*this, random, Tag<float>  {});
        TheTest::run (*this, random, Tag<double> {});
        TheTest::run (*this, random, Tag<int8_t> {});
        TheTest::run (*this, random, Tag<int16_t>{});
        TheTest::run (*this, random, Tag<int32_t>{});
        TheTest::run (*this, random, Tag<int64_t>{});
    }

    void runTest() override
    {
        runTestForAllTypes ("InitializationTest", InitializationTest{});

        runTestForAllTypes ("AccessTest", AccessTest{});

        runTestForAllTypes ("AdditionOperators", OperatorTests<Addition>{});
        runTestForAllTypes ("SubtractionOperators", OperatorTests<Subtraction>{});
        runTestForAllTypes ("MultiplicationOperators", OperatorTests<Multiplication>{});

        runTestForAllTypes ("BitANDOperators", BitOperatorTests<BitAND>{});
        runTestForAllTypes ("BitOROperators", BitOperatorTests<BitOR>{});
        runTestForAllTypes ("BitXOROperators", BitOperatorTests<BitXOR>{});

        runTestNonComplex ("CheckComparisons", CheckComparisonOps{});
        runTestNonComplex ("CheckBoolEquals", CheckBoolEquals{});
        runTestNonComplex ("CheckMinMax", CheckMinMax{});

        runTestForAllTypes ("CheckMultiplyAdd", CheckMultiplyAdd{});
        runTestForAllTypes ("CheckSum", CheckSum{});

        runTestSigned ("CheckAbs", CheckAbs{});

        runTestFloatingPoint ("CheckTruncate", CheckTruncate{});
    }
};

static SIMDRegisterUnitTests SIMDRegisterUnitTests;

} // namespace juce::dsp
