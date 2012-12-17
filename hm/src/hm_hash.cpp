#include "hm_hash.h"
#include "sha1.h"

class Sha1HashImpl
{
public:
    Sha1HashImpl();
    ~Sha1HashImpl();

    void Reset();
    void Update(const void *data, size_t len);
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

void Sha1HashImpl::Update( const void *data, size_t len )
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

void Sha1Hash::Update( const void *data, size_t len )
{
    return mHashImpl->Update(data, len);
}

void Sha1Hash::Final( unsigned char out[20] )
{
    return mHashImpl->Final(out);
}

void Sha1Hash::Reset()
{
    return mHashImpl->Reset();
}

