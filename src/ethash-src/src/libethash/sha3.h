#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "compiler.h"
#include <stdint.h>
#ifndef SHA3_H
#define SHA3_H
#include <stddef.h>
#define SHA3_KECCAK_SPONGE_WORDS \
	(((1600)/8/*bits to byte*/)/sizeof(uint64_t))
	struct ethash_h256;

#define decsha3(bits) \
	int sha3_##bits(uint8_t*, size_t, uint8_t const*, size_t);

	decsha3(256)
		decsha3(512)

		static inline void SHA3_256(struct ethash_h256 const* ret, uint8_t const* data, size_t const size)
	{
		sha3_256((uint8_t*)ret, 32, data, size);
	}

	static inline void SHA3_512(uint8_t* ret, uint8_t const* data, size_t const size)
	{
		sha3_512(ret, 64, data, size);
	}

#ifdef __cplusplus
}
#endif

typedef struct sha3_context_ {
	uint64_t saved;             /* the portion of the input message that we
								 * didn't consume yet */
	union {                     /* Keccak's state */
		uint64_t s[SHA3_KECCAK_SPONGE_WORDS];
		uint8_t sb[SHA3_KECCAK_SPONGE_WORDS * 8];
	};
	unsigned byteIndex;         /* 0..7--the next byte after the set one
								 * (starts from 0; 0--none are buffered) */
	unsigned wordIndex;         /* 0..24--the next word to integrate input
								 * (starts from 0) */
	unsigned capacityWords;     /* the double size of the hash output in
								 * words (e.g. 16 for Keccak 512) */
} sha3_context;

enum SHA3_FLAGS {
	SHA3_FLAGS_NONE = 0,
	SHA3_FLAGS_KECCAK = 1
};

enum SHA3_RETURN {
	SHA3_RETURN_OK = 0,
	SHA3_RETURN_BAD_PARAMS = 1
};
typedef enum SHA3_RETURN sha3_return_t;

/* For Init or Reset call these: */
sha3_return_t sha3_Init(void *priv, unsigned bitSize);

void sha3_Init256(void *priv);
void sha3_Init384(void *priv);
void sha3_Init512(void *priv);

enum SHA3_FLAGS sha3_SetFlags(void *priv, enum SHA3_FLAGS);

void sha3_Update(void *priv, void const *bufIn, size_t len);

void const *sha3_Finalize(void *priv);

/* Single-call hashing */
#ifdef __cplusplus
extern "C"
#endif
sha3_return_t sha3_HashBuffer(
	unsigned bitSize,   /* 256, 384, 512 */
	enum SHA3_FLAGS flags, /* SHA3_FLAGS_NONE or SHA3_FLAGS_KECCAK */
	const void *in, unsigned inBytes,
	void *out, unsigned outBytes);     /* up to bitSize/8; truncation OK */
#endif
