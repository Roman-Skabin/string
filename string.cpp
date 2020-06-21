//
// Copyright 2020 Roman Skabin
//

#define _CRT_SECURE_NO_WARNINGS

#include "string/string.h"
#include <intrin.h>
#include <io.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN 1
    #define VC_EXTRALEAN        1
    #include <Windows.h>
#endif

#define MMX    0
#define SSE    1
#define AVX    2
#define AVX2   3
#define AVX512 4

#if __AVX512BW__ || __AVX512CD__ || __AVX512DQ__ || __AVX512F__ || __AVX512VL__
    #define ISA AVX512
#elif __AVX2__
    #define ISA AVX2
#elif __AVX__
    #define ISA AVX
#elif _M_X64 || _WIN64 || _M_IX86_FP > 0
    #define ISA SSE
#elif _M_IX86 || _WIN32 || _M_IX86_FP == 0
    #define ISA MMX
#else
    #error Undefined ISA
#endif

typedef signed short     s16;
typedef signed long      s32;
typedef signed long long s64;

#define _TO_CSTR(x) #x
#define TO_CSTR(x) _TO_CSTR(x)

#define _CSTRCAT(a, b) a ## b
#define CSTRCAT(a, b) _CSTRCAT(a, b)

#ifdef _DEBUG
    #define ErrorMessage(expr) CSTRCAT(CSTRCAT(CSTRCAT("Check failed [", TO_CSTR(__FILE__)), \
                                               CSTRCAT("(", TO_CSTR(__LINE__))),             \
                                       CSTRCAT(")]: ", TO_CSTR(expr)))

    #define Check(expr)       if (!(expr)) { puts(ErrorMessage(expr)); __debugbreak(); }
    #define DebugResult(expr) Check(expr)
#else
    #define Check(expr)
    #define DebugResult(expr) expr
#endif

static constexpr u64 Align(u64 x)
{
#if ISA >= AVX512
    return ((x + (sizeof(__m512i) - 1)) & ~(sizeof(__m512i) - 1));
#elif ISA >= AVX
    return ((x + (sizeof(__m256i) - 1)) & ~(sizeof(__m256i) - 1));
#elif ISA >= SSE
    return ((x + (sizeof(__m128i) - 1)) & ~(sizeof(__m128i) - 1));
#else
    return ((x + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1));
#endif
}

static constexpr void vmemset(void *dest, char val, u64 bytes)
{
#if ISA >= AVX512
    if (bytes >= sizeof(__m512i))
    {
        __m512i *mm512_dest = static_cast<__m512i *>(dest);
        __m512i  mm512_val  = _mm512_set1_epi8(val);

        while (bytes >= sizeof(__m512i))
        {
            *mm512_dest++ = mm512_val;
            bytes -= sizeof(__m512i);
        }

        dest = mm512_dest;
    }
#endif

#if ISA >= AVX
    if (bytes >= sizeof(__m256i))
    {
        __m256i *mm256_dest = static_cast<__m256i *>(dest);
        __m256i  mm256_val  = _mm256_set1_epi8(val);

        while (bytes >= sizeof(__m256i))
        {
            *mm256_dest++ = mm256_val;
            bytes -= sizeof(__m256i);
        }

        dest = mm256_dest;
    }
#endif

#if ISA >= SSE
    if (bytes >= sizeof(__m128i))
    {
        __m128i *mm128_dest = static_cast<__m128i *>(dest);
        __m128i  mm128_val  = _mm_set1_epi8(val);

        while (bytes >= sizeof(__m128i))
        {
            *mm128_dest++ = mm128_val;
            bytes -= sizeof(__m128i);
        }

        dest = mm128_dest;
    }
#elif ISA >= MMX
    if (bytes >= sizeof(__m64))
    {
        __m64 *m64_dest = static_cast<__m64 *>(dest);
        __m64  m64_val  = _mm_set1_pi8(val);

        while (bytes >= sizeof(__m64))
        {
            *m64_dest++ = m64_val;
            bytes -= sizeof(__m64);
        }

        dest = m64_dest;
    }
#endif

    if (bytes)
    {
        s8 *s8_dest = static_cast<s8 *>(dest);

        while (bytes)
        {
            *s8_dest++ = val;
            --bytes;
        }
    }
}

static constexpr void vmemcpy(void *dest, void *src, u64 bytes)
{
#if ISA >= AVX512
    if (bytes >= sizeof(__m512i))
    {
        __m512i *mm512_dest = static_cast<__m512i *>(dest);
        __m512i *mm512_src  = static_cast<__m512i *>(src);

        while (bytes >= sizeof(__m512i))
        {
            *mm512_dest++ = *mm512_src++;
            bytes -= sizeof(__m512i);
        }

        dest = mm512_dest;
        src  = mm512_src;
    }
#endif

#if ISA >= AVX
    if (bytes >= sizeof(__m256i))
    {
        __m256i *mm256_dest = static_cast<__m256i *>(dest);
        __m256i *mm256_src  = static_cast<__m256i *>(src);

        while (bytes >= sizeof(__m256i))
        {
            *mm256_dest++ = *mm256_src++;
            bytes -= sizeof(__m256i);
        }

        dest = mm256_dest;
        src  = mm256_src;
    }
#endif

#if ISA >= SSE
    if (bytes >= sizeof(__m128i))
    {
        __m128i *mm128_dest = static_cast<__m128i *>(dest);
        __m128i *mm128_src  = static_cast<__m128i *>(src);

        while (bytes >= sizeof(__m128i))
        {
            *mm128_dest++ = *mm128_src++;
            bytes -= sizeof(__m128i);
        }

        dest = mm128_dest;
        src  = mm128_src;
    }
#endif

#if _M_X64 || _WIN64
    if (bytes >= sizeof(s64))
    {
        s64 *s64_dest = static_cast<s64 *>(dest);
        s64 *s64_src  = static_cast<s64 *>(src);

        while (bytes >= sizeof(s64))
        {
            *s64_dest++ = *s64_src++;
            bytes -= sizeof(s64);
        }

        dest = s64_dest;
        src  = s64_src;
    }
#elif ISA >= MMX
    if (bytes >= sizeof(__m64))
    {
        __m64 *m64_dest = static_cast<__m64 *>(dest);
        __m64 *m64_src  = static_cast<__m64 *>(src);

        while (bytes >= sizeof(__m64))
        {
            *m64_dest++ = *m64_src++;
            bytes -= sizeof(__m64);
        }

        dest = m64_dest;
        src  = m64_src;
    }
#endif

    if (bytes >= sizeof(s32))
    {
        s32 *s32_dest = static_cast<s32 *>(dest);
        s32 *s32_src  = static_cast<s32 *>(src);

        while (bytes >= sizeof(s32))
        {
            *s32_dest++ = *s32_src++;
            bytes -= sizeof(s32);
        }

        dest = s32_dest;
        src  = s32_src;
    }

    if (bytes >= sizeof(s16))
    {
        s16 *s16_dest = static_cast<s16 *>(dest);
        s16 *s16_src  = static_cast<s16 *>(src);

        while (bytes >= sizeof(s16))
        {
            *s16_dest++ = *s16_src++;
            bytes -= sizeof(s16);
        }

        dest = s16_dest;
        src  = s16_src;
    }

    if (bytes)
    {
        s8 *s8_dest = static_cast<s8 *>(dest);
        s8 *s8_src  = static_cast<s8 *>(src);

        while (bytes)
        {
            *s8_dest++ = *s8_src++;
            --bytes;
        }
    }
}

String::String()
    : mData(0),
      mLength(0),
      mCapacity(0)
{
}

String::String(u64 capacity)
    : mData(0),
      mLength(0),
      mCapacity(Align(mCapacity))
{
    mData = static_cast<char *>(calloc(1, mCapacity));
}

String::String(char symbol, u64 count)
    : mData(0),
      mLength(count),
      mCapacity(0)
{
    mCapacity = Align(mLength + 1);
    mData     = static_cast<char *>(calloc(1, mCapacity));
    vmemset(mData, symbol, mLength);
}

String::String(const char *cstring)
    : mData(0),
      mLength(strlen(cstring)),
      mCapacity(0)
{
    mCapacity = Align(mLength + 1);
    mData     = static_cast<char *>(calloc(1, mCapacity));
    vmemcpy(mData, const_cast<char *>(cstring), mLength);
}

String::String(const char *cstring, u64 length)
    : mData(0),
      mLength(length),
      mCapacity(0)
{
    mCapacity = Align(mLength + 1);
    mData     = static_cast<char *>(calloc(1, mCapacity));
    vmemcpy(mData, const_cast<char *>(cstring), mLength);
}

String::String(const String& other)
    : mData(0),
      mLength(other.mLength),
      mCapacity(other.mCapacity)
{
    mData = static_cast<char *>(calloc(1, mCapacity));
    vmemcpy(mData, other.mData, mLength);
}

String::String(String&& other) noexcept
    : mData(other.mData),
      mLength(other.mLength),
      mCapacity(other.mCapacity)
{
    other.mData = 0;
}

String::~String()
{
    if (mData)
    {
        free(mData);
        mData = 0;
    }
    mLength   = 0;
    mCapacity = 0;
}

String& String::Clear()
{
    vmemset(mData, '\0', mCapacity);
    return *this;
}

String& String::Reserve(u64 bytes)
{
    bytes = Align(bytes);
    if (bytes > mCapacity)
    {
        mCapacity = bytes;
        mData     = static_cast<char *>(_recalloc(mData, 1, bytes));
    }
    return *this;
}

s8 String::Compare(const String& other) const
{
    if (mLength < other.mLength) return -1;
    if (mLength > other.mLength) return  1;

    const char *left  = mData;
    const char *right = other.mData;

    while (*left++ == *right++)
    {
    }

    if (*left < *right) return -1;
    if (*left > *right) return  1;
    return 0;
}

s8 String::Compare(const char *cstring) const
{
    u64 cstring_length = strlen(cstring);

    if (mLength < cstring_length) return -1;
    if (mLength > cstring_length) return  1;

    const char *left = mData;

    while (*left++ == *cstring++)
    {
    }

    if (*left < *cstring) return -1;
    if (*left > *cstring) return  1;
    return 0;
}

s8 String::Compare(const char *cstring, u64 cstring_length) const
{
    if (mLength < cstring_length) return -1;
    if (mLength > cstring_length) return  1;

    const char *left = mData;

    while (*left++ == *cstring++)
    {
    }

    if (*left < *cstring) return -1;
    if (*left > *cstring) return  1;
    return 0;
}

String& String::Insert(u64 where, const String& other)
{
    Check(where <= mLength);

    u64 old_length = mLength;
    
    mLength += other.mLength;

    if (mLength + 1 >= mCapacity)
    {
        mCapacity = Align(mLength + 1);
        mData     = static_cast<char *>(_recalloc(mData, 1, mCapacity));
    }

    memmove(mData + where + other.mLength, mData + where, old_length - where);
    vmemcpy(mData + where, other.mData, other.mLength);

    return *this;
}

String& String::Insert(u64 where, char symbol)
{
    Check(where <= mLength);

    u64 old_length = mLength;

    mLength += 1;

    if (mLength + 1 >= mCapacity)
    {
        mCapacity = Align(mLength + 1);
        mData     = static_cast<char *>(_recalloc(mData, 1, mCapacity));
    }

    memmove(mData + where + 1, mData + where, old_length - where);
    mData[where] = symbol;

    return *this;
}

String& String::Insert(u64 where, const char *cstring)
{
    Check(where <= mLength);

    u64 old_length     = mLength;
    u64 cstring_length = strlen(cstring);

    mLength += cstring_length;

    if (mLength + 1 >= mCapacity)
    {
        mCapacity = Align(mLength + 1);
        mData     = static_cast<char *>(_recalloc(mData, 1, mCapacity));
    }

    memmove(mData + where + cstring_length, mData + where, old_length - where);
    vmemcpy(mData + where, const_cast<char *>(cstring), cstring_length);

    return *this;
}

String& String::Insert(u64 where, const char *cstring, u64 cstring_length)
{
    Check(where <= mLength);

    u64 old_length = mLength;

    mLength += cstring_length;

    if (mLength + 1 >= mCapacity)
    {
        mCapacity = Align(mLength + 1);
        mData     = static_cast<char *>(_recalloc(mData, 1, mCapacity));
    }

    memmove(mData + where + cstring_length, mData + where, old_length - where);
    vmemcpy(mData + where, const_cast<char *>(cstring), cstring_length);

    return *this;
}

String& String::Erase(u64 from, u64 to)
{
    Check(to > from);
    Check(to <= mLength);

    u64 old_length   = mLength;
    u64 erase_length = to - from;

    mLength -= erase_length;

    memmove(mData + from, mData + to, old_length - to);
    vmemset(mData + mLength, '\0' , erase_length);

    return *this;
}

String String::Concat(const String& left, const String& right)
{
    String result;
    result.mLength   = left.mLength + right.mLength;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData,                left.mData,  left.mLength);
    vmemcpy(result.mData + left.mLength, right.mData, right.mLength);
    return result;
}

String String::Concat(const String& left, String&& right)
{
    return std::move(right.PushFront(left));
}

String String::Concat(const String& left, char right)
{
    String result;
    result.mLength   = left.mLength + 1;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData, left.mData, left.mLength);
    result.mData[left.mLength] = right;
    return result;
}

String String::Concat(const String& left, const char *right)
{
    u64 right_length = strlen(right);
    String result;
    result.mLength   = left.mLength + right_length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData,                left.mData,                left.mLength);
    vmemcpy(result.mData + left.mLength, const_cast<char *>(right), right_length);
    return result;
}

String String::Concat(const String& left, const char *right, u64 right_length)
{
    String result;
    result.mLength   = left.mLength + right_length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData,                left.mData,                left.mLength);
    vmemcpy(result.mData + left.mLength, const_cast<char *>(right), right_length);
    return result;
}

String String::Concat(String&& left, const String& right)
{
    return std::move(left.PushBack(right));
}

String String::Concat(String&& left, char right)
{
    return std::move(left.PushBack(right));
}

String String::Concat(String&& left, const char *right)
{
    return std::move(left.PushBack(right));
}

String String::Concat(String&& left, const char *right, u64 right_length)
{
    return std::move(left.PushBack(right, right_length));
}

String String::Concat(char left, const String& right)
{
    String result;
    result.mLength   = 1 + right.mLength;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    result.mData[0]  = left;
    vmemcpy(result.mData + 1, right.mData, right.mLength);
    return result;
}

String String::Concat(char left, String&& right)
{
    return std::move(right.PushFront(left));
}

String String::Concat(char left, char right)
{
    String result;
    result.mLength   = 2;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    result.mData[0]  = left;
    result.mData[1]  = right;
    return result;
}

String String::Concat(char left, const char *right)
{
    u64 right_length = strlen(right);
    String result;
    result.mLength   = 1 + right_length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    result.mData[0]  = left;
    vmemcpy(result.mData + 1, const_cast<char *>(right), right_length);
    return result;
}

String String::Concat(char left, const char *right, u64 right_length)
{
    String result;
    result.mLength   = 1 + right_length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    result.mData[0]  = left;
    vmemcpy(result.mData + 1, const_cast<char *>(right), right_length);
    return result;
}

String String::Concat(const char *left, const String& right)
{
    u64 left_length = strlen(left);
    String result;
    result.mLength   = left_length + right.mLength;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData,               const_cast<char *>(left), left_length);
    vmemcpy(result.mData + left_length, right.mData,              right.mLength);
    return result;
}

String String::Concat(const char *left, String&& right)
{
    return std::move(right.PushFront(left));
}

String String::Concat(const char *left, char right)
{
    u64 left_length = strlen(left);
    String result;
    result.mLength   = left_length + 1;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData, const_cast<char *>(left), left_length);
    result.mData[left_length] = right;
    return result;
}

String String::Concat(const char *left, const char *right)
{
    u64 left_length  = strlen(left);
    u64 right_length = strlen(right);
    String result;
    result.mLength   = left_length + right_length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData,               const_cast<char *>(left),  left_length);
    vmemcpy(result.mData + left_length, const_cast<char *>(right), right_length);
    return result;
}

String String::Concat(const char *left, const char *right, u64 right_length)
{
    u64 left_length = strlen(left);
    String result;
    result.mLength   = left_length + right_length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData,               const_cast<char *>(left),  left_length);
    vmemcpy(result.mData + left_length, const_cast<char *>(right), right_length);
    return result;
}

String String::Concat(const char *left, u64 left_length, const String& right)
{
    String result;
    result.mLength   = left_length + right.mLength;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData,               const_cast<char *>(left), left_length);
    vmemcpy(result.mData + left_length, right.mData,              right.mLength);
    return result;
}

String String::Concat(const char *left, u64 left_length, String&& right)
{
    return std::move(right.PushFront(left, left_length));
}

String String::Concat(const char *left, u64 left_length, char right)
{
    String result;
    result.mLength   = left_length + 1;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData, const_cast<char *>(left), left_length);
    result.mData[left_length] = right;
    return result;
}

String String::Concat(const char *left, u64 left_length, const char *right)
{
    u64 right_length = strlen(right);
    String result;
    result.mLength   = left_length + right_length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData,               const_cast<char *>(left),  left_length);
    vmemcpy(result.mData + left_length, const_cast<char *>(right), right_length);
    return result;
}

String String::Concat(const char *left, u64 left_length, const char *right, u64 right_length)
{
    String result;
    result.mLength   = left_length + right_length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData,               const_cast<char *>(left),  left_length);
    vmemcpy(result.mData + left_length, const_cast<char *>(right), right_length);
    return result;
}

String String::SubString(u64 from, u64 to) const &
{
    String result;
    result.mLength   = to - from;
    result.mCapacity = Align(result.mLength);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData, mData + from, result.mLength);
    return result;
}

String String::SubString(u64 from, u64 to) &&
{
    u64 sub_len = to - from;
    memmove(mData, mData + from, sub_len);
    vmemset(mData + from + 1, '\0', mLength - (from + 1));
    mLength = sub_len;
    return std::move(*this);
}

String String::SubString(const char *cstring, u64 from, u64 to)
{
    String result;
    result.mLength   = to - from;
    result.mCapacity = Align(result.mLength);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData, const_cast<char *>(cstring) + from, result.mLength);
    return result;
}

String String::Find(const String& string) const &
{
    u64 offset = 0;
    u64 length = 0;

    for (u64 i = 0; i < mLength; ++i)
    {
        if (mLength - i < string.mLength)
        {
            return std::move(String());
        }

        if (mData[i] == *string.mData)
        {
            const char *this_start   = mData + i;
            const char *string_start = string.mData;

            while (*this_start++ == *string_start++)
            {
            }

            if (string_start == '\0')
            {
                offset = i;
                length = this_start - mData - offset;
                break;
            }
        }
    }

    String result;
    result.mLength   = length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData, mData + offset, result.mLength);
    return result;
}

String String::Find(const String& string) &&
{
    u64 offset = 0;
    u64 length = 0;

    for (u64 i = 0; i < mLength; ++i)
    {
        if (mLength - i < string.mLength)
        {
            return std::move(String());
        }

        if (mData[i] == *string.mData)
        {
            const char *this_start   = mData + i;
            const char *string_start = string.mData;

            while (*this_start++ == *string_start++)
            {
            }

            if (string_start == '\0')
            {
                offset = i;
                length = this_start - mData - offset;
                break;
            }
        }
    }

    mLength = length;
    vmemcpy(mData, mData + offset, mLength);
    return std::move(*this);
}

char String::Find(char symbol) const
{
    const char *it = mData;
    while (*it && *it != symbol)
    {
        ++it;
    }
    return *it;
}

String String::Find(const char *cstring) const &
{
    u64 cstring_length = strlen(cstring);
    u64 offset         = 0;
    u64 length         = 0;

    for (u64 i = 0; i < mLength; ++i)
    {
        if (mLength - i < cstring_length)
        {
            return std::move(String());
        }

        if (mData[i] == *cstring)
        {
            const char *this_start   = mData + i;
            const char *string_start = cstring;

            while (*this_start++ == *string_start++)
            {
            }

            if (string_start == '\0')
            {
                offset = i;
                length = this_start - mData - offset;
                break;
            }
        }
    }

    String result;
    result.mLength   = length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData, mData + offset, result.mLength);
    return result;
}

String String::Find(const char *cstring) &&
{
    u64 cstring_length = strlen(cstring);
    u64 offset         = 0;
    u64 length         = 0;

    for (u64 i = 0; i < mLength; ++i)
    {
        if (mLength - i < cstring_length)
        {
            return std::move(String());
        }

        if (mData[i] == *cstring)
        {
            const char *this_start   = mData + i;
            const char *string_start = cstring;

            while (*this_start++ == *string_start++)
            {
            }

            if (string_start == '\0')
            {
                offset = i;
                length = this_start - mData - offset;
                break;
            }
        }
    }

    mLength = length;
    vmemcpy(mData, mData + offset, mLength);
    return std::move(*this);
}

String String::Find(const char *cstring, u64 cstring_length) const &
{
    u64 offset = 0;
    u64 length = 0;

    for (u64 i = 0; i < mLength; ++i)
    {
        if (mLength - i < cstring_length)
        {
            return std::move(String());
        }

        if (mData[i] == *cstring)
        {
            const char *this_start   = mData + i;
            const char *string_start = cstring;

            while (*this_start++ == *string_start++)
            {
            }

            if (string_start == '\0')
            {
                offset = i;
                length = this_start - mData - offset;
                break;
            }
        }
    }

    String result;
    result.mLength   = length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData, mData + offset, result.mLength);
    return result;
}

String String::Find(const char *cstring, u64 cstring_length) &&
{
    u64 offset = 0;
    u64 length = 0;

    for (u64 i = 0; i < mLength; ++i)
    {
        if (mLength - i < cstring_length)
        {
            return std::move(String());
        }

        if (mData[i] == *cstring)
        {
            const char *this_start   = mData + i;
            const char *string_start = cstring;

            while (*this_start++ == *string_start++)
            {
            }

            if (string_start == '\0')
            {
                offset = i;
                length = this_start - mData - offset;
                break;
            }
        }
    }

    mLength = length;
    vmemcpy(mData, mData + offset, mLength);
    return std::move(*this);
}

String String::Find(const char *in_cstring, const String& string)
{
    u64 in_cstring_length = strlen(in_cstring);
    u64 offset            = 0;
    u64 length            = 0;

    for (u64 i = 0; i < in_cstring_length; ++i)
    {
        if (in_cstring_length - i < string.mLength)
        {
            return std::move(String());
        }

        if (in_cstring[i] == *string.mData)
        {
            const char *in_string_start = in_cstring + i;
            const char *string_start    = string.mData;

            while (*in_string_start++ == *string_start++)
            {
            }

            if (string_start == '\0')
            {
                offset = i;
                length = in_string_start - in_cstring - offset;
                break;
            }
        }
    }

    String result;
    result.mLength   = length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData, const_cast<char *>(in_cstring) + offset, result.mLength);
    return result;
}

char String::Find(const char *in_cstring, char symbol)
{
    const char *start = in_cstring;
    while (*start && *start != symbol)
    {
        ++start;
    }
    return *start;
}

String String::Find(const char *in_cstring, const char *cstring)
{
    u64 in_cstring_length = strlen(in_cstring);
    u64 cstring_length    = strlen(cstring);
    u64 offset            = 0;
    u64 length            = 0;

    for (u64 i = 0; i < in_cstring_length; ++i)
    {
        if (in_cstring_length - i < cstring_length)
        {
            return std::move(String());
        }

        if (in_cstring[i] == *cstring)
        {
            const char *in_string_start = in_cstring + i;
            const char *string_start    = cstring;

            while (*in_string_start++ == *string_start++)
            {
            }

            if (string_start == '\0')
            {
                offset = i;
                length = in_string_start - in_cstring - offset;
                break;
            }
        }
    }

    String result;
    result.mLength   = length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData, const_cast<char *>(in_cstring) + offset, result.mLength);
    return result;
}

String String::Find(const char *in_cstring, const char *cstring, u64 cstring_length)
{
    u64 in_cstring_length = strlen(in_cstring);
    u64 offset            = 0;
    u64 length            = 0;

    for (u64 i = 0; i < in_cstring_length; ++i)
    {
        if (in_cstring_length - i < cstring_length)
        {
            return std::move(String());
        }

        if (in_cstring[i] == *cstring)
        {
            const char *in_string_start = in_cstring + i;
            const char *string_start    = cstring;

            while (*in_string_start++ == *string_start++)
            {
            }

            if (string_start == '\0')
            {
                offset = i;
                length = in_string_start - in_cstring - offset;
                break;
            }
        }
    }

    String result;
    result.mLength   = length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData, const_cast<char *>(in_cstring) + offset, result.mLength);
    return result;
}

String String::Find(const char *in_cstring, u64 in_cstring_length, const String& string)
{
    u64 offset = 0;
    u64 length = 0;

    for (u64 i = 0; i < in_cstring_length; ++i)
    {
        if (in_cstring_length - i < string.mLength)
        {
            return std::move(String());
        }

        if (in_cstring[i] == *string.mData)
        {
            const char *in_string_start = in_cstring + i;
            const char *string_start    = string.mData;

            while (*in_string_start++ == *string_start++)
            {
            }

            if (string_start == '\0')
            {
                offset = i;
                length = in_string_start - in_cstring - offset;
                break;
            }
        }
    }

    String result;
    result.mLength   = length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData, const_cast<char *>(in_cstring) + offset, result.mLength);
    return result;
}

char String::Find(const char *in_cstring, u64 in_cstring_length, char symbol)
{
    const char *start = in_cstring;
    const char *end   = in_cstring + in_cstring_length;
    while (start < end && *start != symbol)
    {
        ++start;
    }
    return *start;
}

String String::Find(const char *in_cstring, u64 in_cstring_length, const char *cstring)
{
    u64 cstring_length = strlen(cstring);
    u64 offset         = 0;
    u64 length         = 0;

    for (u64 i = 0; i < in_cstring_length; ++i)
    {
        if (in_cstring_length - i < cstring_length)
        {
            return std::move(String());
        }

        if (in_cstring[i] == *cstring)
        {
            const char *in_string_start = in_cstring + i;
            const char *string_start    = cstring;

            while (*in_string_start++ == *string_start++)
            {
            }

            if (string_start == '\0')
            {
                offset = i;
                length = in_string_start - in_cstring - offset;
                break;
            }
        }
    }

    String result;
    result.mLength   = length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData, const_cast<char *>(in_cstring) + offset, result.mLength);
    return result;
}

String String::Find(const char *in_cstring, u64 in_cstring_length, const char *cstring, u64 cstring_length)
{
    u64 offset = 0;
    u64 length = 0;

    for (u64 i = 0; i < in_cstring_length; ++i)
    {
        if (in_cstring_length - i < cstring_length)
        {
            return std::move(String());
        }

        if (in_cstring[i] == *cstring)
        {
            const char *in_string_start = in_cstring + i;
            const char *string_start    = cstring;

            while (*in_string_start++ == *string_start++)
            {
            }

            if (string_start == '\0')
            {
                offset = i;
                length = in_string_start - in_cstring - offset;
                break;
            }
        }
    }

    String result;
    result.mLength   = length;
    result.mCapacity = Align(result.mLength + 1);
    result.mData     = static_cast<char *>(calloc(1, result.mCapacity));
    vmemcpy(result.mData, const_cast<char *>(in_cstring) + offset, result.mLength);
    return result;
}

const String& String::WriteToFile(int unix_file, bool binary) const
{
    if (binary)
    {
        DebugResult(_write(unix_file, &mLength, sizeof(u64)) != -1);
    }
    DebugResult(_write(unix_file, mData, static_cast<int>(mLength)) != -1);
    return *this;
}

const String& String::WriteToFile(void *win_file, bool binary) const
{
#ifdef _WIN32
    if (binary)
    {
        DebugResult(WriteFile(win_file, &mLength, sizeof(u64), 0, 0) != -1);
    }
    DebugResult(WriteFile(win_file, mData, static_cast<int>(mLength), 0, 0));
#endif
    return *this;
}

const String& String::WriteToFile(FILE *crt_file, bool binary) const
{
    if (binary)
    {
        fwrite(&mLength, sizeof(u64), 1, crt_file);
    }
    fwrite(mData, mLength, 1, crt_file);
    return *this;
}

const String& String::WriteToFile(const char *filename, bool binary) const
{
    FILE *crt_file = 0;
    if (binary)
    {
        crt_file = fopen(filename, "wb");
        Check(crt_file);
        fwrite(&mLength, sizeof(u64), 1, crt_file);
    }
    else
    {
        crt_file = fopen(filename, "wt");
        Check(crt_file);
    }
    fwrite(mData, mLength, 1, crt_file);
    fclose(crt_file);
    return *this;
}

const String& String::WriteToFile(const String& filename, bool binary) const
{
    FILE *crt_file = 0;
    if (binary)
    {
        crt_file = fopen(filename.mData, "wb");
        Check(crt_file);
        fwrite(&mLength, sizeof(u64), 1, crt_file);
    }
    else
    {
        crt_file = fopen(filename.mData, "wt");
        Check(crt_file);
    }
    fwrite(mData, mLength, 1, crt_file);
    fclose(crt_file);
    return *this;
}

String& String::WriteToFile(int unix_file, bool binary)
{
    if (binary)
    {
        DebugResult(_write(unix_file, &mLength, sizeof(u64)) != -1);
    }
    DebugResult(_write(unix_file, mData, static_cast<int>(mLength)) != -1);
    return *this;
}

String& String::WriteToFile(void *win_file, bool binary)
{
#ifdef _WIN32
    if (binary)
    {
        DebugResult(WriteFile(win_file, &mLength, sizeof(u64), 0, 0) != -1);
    }
    DebugResult(WriteFile(win_file, mData, static_cast<int>(mLength), 0, 0));
#endif
    return *this;
}

String& String::WriteToFile(FILE *crt_file, bool binary)
{
    if (binary)
    {
        fwrite(&mLength, sizeof(u64), 1, crt_file);
    }
    fwrite(mData, mLength, 1, crt_file);
    return *this;
}

String& String::WriteToFile(const char *filename, bool binary)
{
    FILE *crt_file = 0;
    if (binary)
    {
        crt_file = fopen(filename, "wb");
        Check(crt_file);
        fwrite(&mLength, sizeof(u64), 1, crt_file);
    }
    else
    {
        crt_file = fopen(filename, "wt");
        Check(crt_file);
    }
    fwrite(mData, mLength, 1, crt_file);
    fclose(crt_file);
    return *this;
}

String& String::WriteToFile(const String& filename, bool binary)
{
    FILE *crt_file = 0;
    if (binary)
    {
        crt_file = fopen(filename.mData, "wb");
        Check(crt_file);
        fwrite(&mLength, sizeof(u64), 1, crt_file);
    }
    else
    {
        crt_file = fopen(filename.mData, "wt");
        Check(crt_file);
    }
    fwrite(mData, mLength, 1, crt_file);
    fclose(crt_file);
    return *this;
}

String& String::ReadFromFile(int unix_file, u64 num_chars_to_read, bool binary)
{
    u64 old_len = mLength;

    if (binary)
    {
        DebugResult(_read(unix_file, &mLength, sizeof(u64)) != -1);
    }
    else
    {
        mLength = num_chars_to_read;
    }

    if (mLength >= mCapacity)
    {
        mCapacity = Align(mLength + 1);
        mData     = static_cast<char *>(_recalloc(mData, 1, mCapacity));
    }
    else if (mLength < old_len)
    {
        vmemset(mData + mLength, '\0', old_len - mLength);
    }

    DebugResult(_read(unix_file, mData, static_cast<int>(mLength)) != -1);

    return *this;
}

String& String::ReadFromFile(void *win_file, u64 num_chars_to_read, bool binary)
{
    u64 old_len = mLength;

    if (binary)
    {
        DebugResult(ReadFile(win_file, &mLength, sizeof(u64), 0, 0));
    }
    else
    {
        mLength = num_chars_to_read;
    }

    if (mLength >= mCapacity)
    {
        mCapacity = Align(mLength + 1);
        mData     = static_cast<char *>(_recalloc(mData, 1, mCapacity));
    }
    else if (mLength < old_len)
    {
        vmemset(mData + mLength, '\0', old_len - mLength);
    }

    DebugResult(ReadFile(win_file, mData, static_cast<int>(mLength), 0, 0));

    return *this;
}

String& String::ReadFromFile(FILE *crt_file, u64 num_chars_to_read, bool binary)
{
    u64 old_len = mLength;

    if (binary)
    {
        fread(&mLength, sizeof(u64), 1, crt_file);
    }
    else
    {
        mLength = num_chars_to_read;
    }

    if (mLength >= mCapacity)
    {
        mCapacity = Align(mLength + 1);
        mData     = static_cast<char *>(_recalloc(mData, 1, mCapacity));
    }
    else if (mLength < old_len)
    {
        vmemset(mData + mLength, '\0', old_len - mLength);
    }

    fread(mData, mLength, 1, crt_file);

    return *this;
}

String& String::ReadFromFile(const char *filename, u64 num_chars_to_read, bool binary)
{
    FILE *crt_file = 0;
    u64   old_len  = mLength;

    if (binary)
    {
        DebugResult(crt_file = fopen(filename, "rb"));
        fread(&mLength, sizeof(u64), 1, crt_file);
    }
    else
    {
        DebugResult(crt_file = fopen(filename, "rt"));
        mLength = num_chars_to_read;
    }

    if (mLength >= mCapacity)
    {
        mCapacity = Align(mLength + 1);
        mData     = static_cast<char *>(_recalloc(mData, 1, mCapacity));
    }
    else if (mLength < old_len)
    {
        vmemset(mData + mLength, '\0', old_len - mLength);
    }

    fread(mData, mLength, 1, crt_file);
    fclose(crt_file);
    return *this;
}

String& String::ReadFromFile(const String& filename, u64 num_chars_to_read, bool binary)
{
    FILE *crt_file = 0;
    u64   old_len  = mLength;

    if (binary)
    {
        DebugResult(crt_file = fopen(filename.mData, "rb"));
        fread(&mLength, sizeof(u64), 1, crt_file);
    }
    else
    {
        DebugResult(crt_file = fopen(filename.mData, "rt"));
        mLength = num_chars_to_read;
    }

    if (mLength >= mCapacity)
    {
        mCapacity = Align(mLength + 1);
        mData     = static_cast<char *>(_recalloc(mData, 1, mCapacity));
    }
    else if (mLength < old_len)
    {
        vmemset(mData + mLength, '\0', old_len - mLength);
    }

    fread(mData, mLength, 1, crt_file);
    fclose(crt_file);
    return *this;
}

const String& String::AppendToFile(const char *filename, bool binary) const
{
    FILE *crt_file = 0;

    if (binary)
    {
        DebugResult(crt_file = fopen(filename, "ab"));
        fwrite(&mLength, sizeof(u64), 1, crt_file);
    }
    else
    {
        DebugResult(crt_file = fopen(filename, "at"));
    }

    fwrite(mData, mLength, 1, crt_file);
    fclose(crt_file);

    return *this;
}

const String& String::AppendToFile(const String& filename, bool binary) const
{
    FILE *crt_file = 0;

    if (binary)
    {
        DebugResult(crt_file = fopen(filename.mData, "ab"));
        fwrite(&mLength, sizeof(u64), 1, crt_file);
    }
    else
    {
        DebugResult(crt_file = fopen(filename.mData, "at"));
    }

    fwrite(mData, mLength, 1, crt_file);
    fclose(crt_file);

    return *this;
}

String& String::AppendToFile(const char *filename, bool binary)
{
    FILE *crt_file = 0;

    if (binary)
    {
        DebugResult(crt_file = fopen(filename, "ab"));
        fwrite(&mLength, sizeof(u64), 1, crt_file);
    }
    else
    {
        DebugResult(crt_file = fopen(filename, "at"));
    }

    fwrite(mData, mLength, 1, crt_file);
    fclose(crt_file);

    return *this;
}

String& String::AppendToFile(const String& filename, bool binary)
{
    FILE *crt_file = 0;

    if (binary)
    {
        DebugResult(crt_file = fopen(filename.mData, "ab"));
        fwrite(&mLength, sizeof(u64), 1, crt_file);
    }
    else
    {
        DebugResult(crt_file = fopen(filename.mData, "at"));
    }

    fwrite(mData, mLength, 1, crt_file);
    fclose(crt_file);

    return *this;
}

char String::operator[](u64 index) const
{
    Check(index < mLength);
    return static_cast<char *>(mData)[index];
}

char& String::operator[](u64 index)
{
    Check(index < mLength);
    return static_cast<char *>(mData)[index];
}

String& String::operator=(const String& other)
{
    if (&other != this)
    {
        if (other.mLength < mCapacity)
        {
            if (mLength > other.mLength)
            {
                vmemset(mData + other.mLength + 1, '\0', mLength - (other.mLength + 1));
            }
            vmemcpy(mData, other.mData, other.mLength);
            mLength = other.mLength;
        }
        else
        {
            mLength   = other.mLength;
            mCapacity = Align(mLength + 1);
            mData     = static_cast<char *>(_recalloc(mData, 1, mCapacity));
            vmemcpy(mData, other.mData, other.mLength);
        }
    }
    return *this;
}

String& String::operator=(String&& other) noexcept
{
    if (&other != this)
    {
        if (mData) free(mData);

        mData     = other.mData;
        mLength   = other.mLength;
        mCapacity = other.mCapacity;

        other.mData = 0;
    }
    return *this;
}

String& String::operator=(char symbol)
{
    if (mData)
    {
        mData[0] = symbol;
        vmemset(mData + 1, '\0', mLength - 1);
        mLength  = 1;
    }
    else
    {
        mLength   = 1;
        mCapacity = Align(mLength + 1);
        mData     = static_cast<char *>(calloc(1, mCapacity));
        mData[0]  = symbol;
    }
    return *this;
}

String& String::operator=(const char *cstring)
{
    u64 cstring_length = strlen(cstring);
    if (cstring_length < mCapacity)
    {
        if (mLength > cstring_length)
        {
            vmemset(mData + cstring_length + 1, '\0', mLength - (cstring_length + 1));
        }
        vmemcpy(mData, const_cast<char *>(cstring), cstring_length);
        mLength = cstring_length;
    }
    else
    {
        mLength   = cstring_length;
        mCapacity = Align(mLength + 1);
        mData     = static_cast<char *>(_recalloc(mData, 1, mCapacity));
        vmemcpy(mData, const_cast<char *>(cstring), mLength);
    }
    return *this;
}
