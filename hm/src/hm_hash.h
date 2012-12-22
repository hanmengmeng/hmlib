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
    void Update(const void *data, t_size len);
    void Final(unsigned char out[HASH_SHA1_LEN]);

    static std::string ToString(const object_id &oid);
private:
    Sha1HashImpl *mHashImpl;
};

} // namespace hm

#endif

