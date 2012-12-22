#include "hm_hash.h"
extern "C"
{
#include "sha1.h"
};

namespace hm
{

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

std::string Sha1Hash::ToString(const object_id &oid)
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

} // namespace hm

