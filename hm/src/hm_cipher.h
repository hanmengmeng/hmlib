#ifndef HM_CIPHER_H
#define HM_CIPHER_H

#include "openssl/evp.h"

#define CIPHER_BUFFER_SIZE 4096

int CipherInit(EVP_CIPHER_CTX **octx, const unsigned char *key, int type);
int CipherUpdate(EVP_CIPHER_CTX *ctx, unsigned char *outbuf,
                size_t *outlen, const unsigned char *inbuf,
                size_t inlen, size_t leftsize);
int CipherUpdateProcess(EVP_CIPHER_CTX *ctx, unsigned char *outbuf,
                size_t *outlen, const unsigned char *inbuf,
                size_t inlen);
int CipherUpdateFinal(EVP_CIPHER_CTX *ctx, unsigned char *outbuf, size_t *outlen);
void CipherCleanup(EVP_CIPHER_CTX *ctx);
size_t CalculateEncryptedFileSize(size_t size);

#endif
