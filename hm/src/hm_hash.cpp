#include "hm_hash.h"
#include "hm_string.h"
#include <assert.h>

extern "C"
{
#include "sha1.h"
};

using namespace hm;

class Sha1HashImpl
{
public:
    Sha1HashImpl();
    ~Sha1HashImpl();

    void Reset();
    void Update(const void *data, t_size len);
    void Final(unsigned char out[20]);

private:
    SHA_CTX mCtx;
};

Sha1HashImpl::Sha1HashImpl()
{
    SHA1_Init(&mCtx);
}

Sha1HashImpl::~Sha1HashImpl()
{

}

void Sha1HashImpl::Update( const void *data, t_size len )
{
    SHA1_Update(&mCtx, data, len);
}

void Sha1HashImpl::Final( unsigned char out[20] )
{
    SHA1_Final(out, &mCtx);
}

void Sha1HashImpl::Reset()
{
    SHA1_Init(&mCtx);
}


Sha1Hash::Sha1Hash()
{
    mHashImpl = new Sha1HashImpl();
}

Sha1Hash::~Sha1Hash()
{
    delete mHashImpl;
}

void Sha1Hash::Update( const void *data, t_size len )
{
    return mHashImpl->Update(data, len);
}

void Sha1Hash::Final( unsigned char out[HASH_SHA1_LEN] )
{
    return mHashImpl->Final(out);
}

void Sha1Hash::Reset()
{
    return mHashImpl->Reset();
}

std::string Sha1Hash::OidToString(const object_id &oid)
{
    char to_hex[] = "0123456789abcdef";
    char str[HASH_SHA1_STRING_LEN+1];
    char *p = str;

    t_size i = 0;
    for (i = 0; i < sizeof(oid.id); i++)
    {
        *p++ = to_hex[oid.id[i] >> 4];
        *p++ = to_hex[oid.id[i] & 0xf];
    }
    *p = '\0';

    return str;
}

static char from_hex(char c)
{
    char h = 0;
    switch (c)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        h = c - '0';
        break;
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
        h = c - 'a' + 10;
        break;
    default:
        assert(0);
        break;
    }
    return h;
}

object_id Sha1Hash::StringToOid( const std::string &strOid )
{
    assert(strOid.length() >= sizeof(object_id)*2);
    std::string str = strOid;
    ToLower(str);

    object_id ret;
    for (t_size i = 0, j = 0; i < sizeof(ret.id); i++, j+=2)
    {
        ret.id[i] |= 0xff;
        ret.id[i] &= (from_hex(str.at(j)) << 4);
        ret.id[i] |= from_hex(str.at(j+1));
    }
    return ret;
}


