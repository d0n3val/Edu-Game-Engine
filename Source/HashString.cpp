#include "HashString.h"

#define  STRPOOL_IMPLEMENTATION

#include "utils/strpool.h"

namespace
{

    struct PoolHolder
    {
        strpool_t pool;

        PoolHolder()
        {
            strpool_config_t conf = strpool_default_config;
            conf.ignore_case = true;

            strpool_init(&pool, &conf);
        }

        ~PoolHolder()
        {
            strpool_term(&pool);
        }
    };

}

HashString::HashString() : handle(0)
{
}

HashString::HashString(const char* str)
{
    strpool_t* pool = GetPool();

    handle = strpool_inject(pool, str, (int)strlen(str));
    strpool_incref(pool, handle);
    value = strpool_cstr(GetPool(), handle);
}

HashString::HashString(const HashString& rhs) : handle(rhs.handle)
{
    strpool_incref(GetPool(), handle);
	value = strpool_cstr(GetPool(), handle);
}

HashString::~HashString()
{
	strpool_t* pool = GetPool();
	
	if(strpool_decref(pool, handle) == 0)
    {
        strpool_discard(pool, handle);
    }
}

HashString& HashString::operator=(const HashString& rhs)
{
    strpool_t* pool = GetPool();

    strpool_incref(pool, rhs.handle);

    if(strpool_decref(pool, handle) == 0)
    {
        strpool_discard(pool, handle);
    }

    handle = rhs.handle;
    value  = rhs.value;

	return *this;
}

const char* HashString::C_str() const
{
    return strpool_cstr(GetPool(), handle);
}

int HashString::Length() const
{
    return strpool_length(GetPool(), handle);
}

strpool_t* HashString::GetPool() const
{
    static PoolHolder holder;

    return &holder.pool;
}
