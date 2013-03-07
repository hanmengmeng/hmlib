#ifndef HM_HASH_H
#define HM_HASH_H

#include "hm_common.h"

class Sha1HashImpl;

namespace hm
{

class Sha1Hash
{
public:
    Sha1Hash();
    ~Sha1Hash();

    void Reset();
    void Update(const void *data, t_size len);
    void Final(unsigned char out[HASH_SHA1_LEN]);

    static std::string OidToString(const object_id &oid);
    static object_id StringToOid(const std::string &strOid);
private:
    Sha1HashImpl *mHashImpl;
};

} // namespace hm

#endif

