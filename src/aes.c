#include "garble/aes.h"

extern inline void
AES_set_encrypt_key(const block userkey, AES_KEY *key);

extern inline void
AES_ecb_encrypt_blks(block *blks, unsigned nblks, const AES_KEY *key);

extern inline void
AES_set_decrypt_key_fast(AES_KEY *dkey, const AES_KEY *ekey);

extern inline void
AES_ecb_decrypt_blks(block *blks, unsigned nblks, const AES_KEY *key);
