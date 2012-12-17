#ifndef HM_HASH_H
#define HM_HASH_H

class Sha1HashImpl;
class Sha1Hash
{
public:
    Sha1Hash();
    ~Sha1Hash();

    void Reset();
    void Update(const void *data, size_t len);
    void Final(unsigned char out[20]);

private:
    Sha1HashImpl *mHashImpl;
};

#endif

