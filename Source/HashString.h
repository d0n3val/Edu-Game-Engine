#ifndef _HASHSTRING_H_
#define _HASHSTRING_H_

#include "utils/strpool.h"

class HashString
{

    STRPOOL_U64 handle = 0;
    const char* value  = nullptr;

public:

    HashString();
    explicit HashString(const char* str);
    HashString(const char* str, uint len);
    HashString(const HashString& rhs);
    ~HashString();

    HashString& operator=(const HashString& rhs);

	operator bool() const;
    bool operator<(const HashString& rhs) const;
    bool operator>(const HashString& rhs) const;
    bool operator>=(const HashString& rhs) const;
    bool operator<=(const HashString& rhs) const;
    bool operator==(const HashString& rhs) const;
    bool operator!=(const HashString& rhs) const;

    const char* C_str() const;
    int Length() const;
private:

    strpool_t* GetPool() const;

};

inline bool HashString::operator<(const HashString& rhs) const
{
    return handle < rhs.handle;
}

inline bool HashString::operator>(const HashString& rhs) const
{
    return handle > rhs.handle;
}

inline bool HashString::operator>=(const HashString& rhs) const
{
    return handle >= rhs.handle;
}

inline bool HashString::operator<=(const HashString& rhs) const
{
    return handle <= rhs.handle;
}

inline bool HashString::operator==(const HashString& rhs) const
{
    return handle == rhs.handle;
}

inline bool HashString::operator!=(const HashString& rhs) const
{
    return handle != rhs.handle;
}

inline HashString::operator bool() const
{
	return handle != 0;
}

#endif /* _HASHSTRING_H_ */
