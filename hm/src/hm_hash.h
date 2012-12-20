#ifndef HM_HASH_H
#define HM_HASH_H

#include "hm_common.h"

namespace hm
{

class Sha1HashImpl;
class Sha1Hash
{
public:
    Sha1Hash();
    ~Sha1Hash();

    void Reset();
    void Update(const void *data, size_t len);
    void Final(unsigned char out[HASH_SHA1_LEN]);

private:
    Sha1HashImpl *mHashImpl;
};

} // namespace hm

#endif

