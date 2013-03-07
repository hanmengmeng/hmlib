#include "hm_cipher.h"
#include <string.h>

int CipherInit(EVP_CIPHER_CTX **octx, const unsigned char *key, int type)
{
    unsigned char iv[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));
    unsigned char aesKey[32];

    memset(aesKey, 0, 32);
    strcpy((char*)aesKey, (char*)key);
    EVP_CIPHER_CTX_init(ctx);
    if (!EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, aesKey, iv, type))
    {
        return -1;
    }

    *octx = ctx;
    return 0;
}

int CipherUpdate(EVP_CIPHER_CTX *ctx, unsigned char *outbuf, 
    size_t *outlen, const unsigned char *inbuf, 
    size_t inlen, size_t leftsize)
{
    int cipher_len = 0;
    int extra = 0;

    if(!EVP_CipherUpdate(ctx, outbuf, &cipher_len, inbuf, inlen))
    {
        /* Error */
        return -1;
    }
    if (leftsize == 0)
    {
        if (!EVP_CipherFinal_ex(ctx, outbuf+cipher_len, &extra))
        {
            return -1;
        }

        cipher_len+=extra;
    }
    *outlen = cipher_len;
    return 0;
}

void CipherCleanup(EVP_CIPHER_CTX *ctx)
{
    EVP_CIPHER_CTX_cleanup(ctx);
    free(ctx);
}

size_t CalculateEncryptedFileSize( size_t length )
{
    // calculate the encrypted file size
    size_t tail = length % CIPHER_BUFFER_SIZE;
    if (tail%16 == 0)
    {
        length = length + 16;
    }
    else
    {
        length = length + (tail/16 + (tail%16 > 0 ? 1 : 0))*16 - tail;
    }

    return length;
}

int CipherUpdateProcess( EVP_CIPHER_CTX *ctx, unsigned char *outbuf, size_t *outlen, const unsigned char *inbuf, size_t inlen )
{
    int cipher_len = 0;

    if(!EVP_CipherUpdate(ctx, outbuf, &cipher_len, inbuf, inlen))
    {
        /* Error */
        return -1;
    }

    *outlen = cipher_len;
    return 0;
}

int CipherUpdateFinal( EVP_CIPHER_CTX *ctx, unsigned char *outbuf, size_t *outlen )
{
    int extra = 0;
    if (!EVP_CipherFinal_ex(ctx, outbuf, &extra))
    {
        return -1;
    }
    *outlen = extra;
    return 0;
}
