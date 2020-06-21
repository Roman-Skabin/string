//
// Copyright 2020 Roman Skabin
//

#pragma once

#include <type_traits>
#include <stdio.h>

typedef signed char        s8;
typedef unsigned long long u64;

class String
{
public:
    String();
    String(u64 capacity);
    String(char symbol, u64 count);
    String(const char *cstring);
    String(const char *cstring, u64 length);
    String(const String& other);
    String(String&& other) noexcept;

    ~String();

    String& Clear();

    String& Reserve(u64 bytes);

    operator const char *() const { return static_cast<const char *>(mData); }
    operator       char *()       { return mData;                            }

    u64 Length()   const { return mLength;   }
    u64 Capacity() const { return mCapacity; }

    // @NOTE(Roman): -1 - less
    //                0 - equals
    //                1 - greater
    s8 Compare(const String& other) const;
    s8 Compare(const char *cstring) const;
    s8 Compare(const char *cstring, u64 cstring_length) const;

    bool Equals(const String& other)                     const { return !Compare(other);                   }
    bool Equals(const char *cstring)                     const { return !Compare(cstring);                 }
    bool Equals(const char *cstring, u64 cstring_length) const { return !Compare(cstring, cstring_length); }

    String& Insert(u64 where, const String& other);
    String& Insert(u64 where,       char    symbol);
    String& Insert(u64 where, const char   *cstring);
    String& Insert(u64 where, const char   *cstring, u64 cstring_length);

    String& Erase(u64 from, u64 to);

    String& PushBack(const String& other)                     { return Insert(mLength, other);                   }
    String& PushBack(      char  symbol)                      { return Insert(mLength, symbol);                  }
    String& PushBack(const char *cstring)                     { return Insert(mLength, cstring);                 }
    String& PushBack(const char *cstring, u64 cstring_length) { return Insert(mLength, cstring, cstring_length); }

    String& PushFront(const String& other)                     { return Insert(0, other);                   }
    String& PushFront(      char  symbol)                      { return Insert(0, symbol);                  }
    String& PushFront(const char *cstring)                     { return Insert(0, cstring);                 }
    String& PushFront(const char *cstring, u64 cstring_length) { return Insert(0, cstring, cstring_length); }

    static String Concat(const String&  left, const String&  right);
    static String Concat(const String&  left,       String&& right);
    static String Concat(const String&  left,       char     right);
    static String Concat(const String&  left, const char    *right);
    static String Concat(const String&  left, const char    *right, u64 right_length);
    static String Concat(      String&& left, const String&  right);
    static String Concat(      String&& left,       char     right);
    static String Concat(      String&& left, const char    *right);
    static String Concat(      String&& left, const char    *right, u64 right_length);
    static String Concat(      char     left, const String&  right);
    static String Concat(      char     left,       String&& right);
    static String Concat(      char     left,       char     right);
    static String Concat(      char     left, const char    *right);
    static String Concat(      char     left, const char    *right, u64 right_length);
    static String Concat(const char    *left, const String&  right);
    static String Concat(const char    *left,       String&& right);
    static String Concat(const char    *left,       char     right);
    static String Concat(const char    *left, const char    *right);
    static String Concat(const char    *left, const char    *right, u64 right_length);
    static String Concat(const char    *left, u64 left_length, const String&  right);
    static String Concat(const char    *left, u64 left_length,       String&& right);
    static String Concat(const char    *left, u64 left_length,       char     right);
    static String Concat(const char    *left, u64 left_length, const char    *right);
    static String Concat(const char    *left, u64 left_length, const char    *right, u64 right_length);

    String SubString(u64 from, u64 to) const &;
    String SubString(u64 from, u64 to) &&;

    static String SubString(const char *cstring, u64 from, u64 to);

    String Find(const String& string) const &;
    String Find(const String& string) &&;
    char   Find(      char    symbol) const;
    String Find(const char   *cstring) const &;
    String Find(const char   *cstring) &&;
    String Find(const char   *cstring, u64 cstring_length) const &;
    String Find(const char   *cstring, u64 cstring_length) &&;

    static String Find(const char *in_cstring, const String& string);
    static char   Find(const char *in_cstring,       char    symbol);
    static String Find(const char *in_cstring, const char   *cstring);
    static String Find(const char *in_cstring, const char   *cstring, u64 cstring_length);
    static String Find(const char *in_cstring, u64 in_cstring_length, const String& string);
    static char   Find(const char *in_cstring, u64 in_cstring_length,       char    symbol);
    static String Find(const char *in_cstring, u64 in_cstring_length, const char   *cstring);
    static String Find(const char *in_cstring, u64 in_cstring_length, const char   *cstring, u64 cstring_length);

    const String& WriteToFile(      int     unix_file, bool binary = false) const;
    const String& WriteToFile(      void   *win_file,  bool binary = false) const;
    const String& WriteToFile(      FILE   *crt_file,  bool binary = false) const;
    const String& WriteToFile(const char   *filename,  bool binary = false) const;
    const String& WriteToFile(const String& filename,  bool binary = false) const;
          String& WriteToFile(      int     unix_file, bool binary = false);
          String& WriteToFile(      void   *win_file,  bool binary = false);
          String& WriteToFile(      FILE   *crt_file,  bool binary = false);
          String& WriteToFile(const char   *filename,  bool binary = false);
          String& WriteToFile(const String& filename,  bool binary = false);

    // @NOTE(Roman): Param num_chars_to_read needs only if you are _not_ using binary read.
    String& ReadFromFile(      int     unix_file, u64 num_chars_to_read, bool binary = false);
    String& ReadFromFile(      void   *win_file,  u64 num_chars_to_read, bool binary = false);
    String& ReadFromFile(      FILE   *crt_file,  u64 num_chars_to_read, bool binary = false);
    String& ReadFromFile(const char   *filename,  u64 num_chars_to_read, bool binary = false);
    String& ReadFromFile(const String& filename,  u64 num_chars_to_read, bool binary = false);

    const String& AppendToFile(const char   *filename, bool binary = false) const;
    const String& AppendToFile(const String& filename, bool binary = false) const;
          String& AppendToFile(const char   *filename, bool binary = false);
          String& AppendToFile(const String& filename, bool binary = false);

    String& operator+=(const String& right) { return PushBack(right); }
    String& operator+=(      char    right) { return PushBack(right); }
    String& operator+=(const char   *right) { return PushBack(right); }

    char  operator[](u64 index) const;
    char& operator[](u64 index);

    String& operator=(const String&  other);
    String& operator=(      String&& other) noexcept;
    String& operator=(      char     symbol);
    String& operator=(const char    *cstring);

private:
    char *mData;
    u64   mLength;
    u64   mCapacity;
};

inline bool operator==(const String& left, const String& right) { return !left.Compare(right); }
inline bool operator==(const String& left, const char   *right) { return !left.Compare(right); }
inline bool operator==(const char   *left, const String& right) { return !right.Compare(left); }

inline bool operator!=(const String& left, const String& right) { return left.Compare(right); }
inline bool operator!=(const String& left, const char   *right) { return left.Compare(right); }
inline bool operator!=(const char   *left, const String& right) { return right.Compare(left); }

inline bool operator<(const String& left, const String& right) { return left.Compare(right) < 0; }
inline bool operator<(const String& left, const char   *right) { return left.Compare(right) < 0; }
inline bool operator<(const char   *left, const String& right) { return right.Compare(left) > 0; }

inline bool operator<=(const String& left, const String& right) { return left.Compare(right) <= 0; }
inline bool operator<=(const String& left, const char   *right) { return left.Compare(right) <= 0; }
inline bool operator<=(const char   *left, const String& right) { return right.Compare(left) >= 0; }

inline bool operator>(const String& left, const String& right) { return left.Compare(right) > 0; }
inline bool operator>(const String& left, const char   *right) { return left.Compare(right) > 0; }
inline bool operator>(const char   *left, const String& right) { return right.Compare(left) < 0; }

inline bool operator>=(const String& left, const String& right) { return left.Compare(right) >= 0; }
inline bool operator>=(const String& left, const char   *right) { return left.Compare(right) >= 0; }
inline bool operator>=(const char   *left, const String& right) { return right.Compare(left) <= 0; }

inline String operator+(const String&  left, const String&  right) { return String::Concat(          left,            right ); }
inline String operator+(const String&  left,       String&& right) { return String::Concat(          left,  std::move(right)); }
inline String operator+(const String&  left,       char     right) { return String::Concat(          left,            right ); }
inline String operator+(const String&  left, const char    *right) { return String::Concat(          left,            right ); }
inline String operator+(      String&& left, const String&  right) { return String::Concat(std::move(left),           right ); }
inline String operator+(      String&& left,       char     right) { return String::Concat(std::move(left),           right ); }
inline String operator+(      String&& left, const char    *right) { return String::Concat(std::move(left),           right ); }
inline String operator+(      char     left, const String&  right) { return String::Concat(          left,            right ); }
inline String operator+(      char     left,       String&& right) { return String::Concat(          left,  std::move(right)); }
inline String operator+(const char    *left, const String&  right) { return String::Concat(          left,            right ); }
inline String operator+(const char    *left,       String&& right) { return String::Concat(          left,  std::move(right)); }
