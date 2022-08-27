#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <stdint.h>
#include <nan.h>
#include <stdexcept>

#include "endian.h"
#include "util.h"
#include "messages.h"
#include "libethash/ethash.h"
#include "libethash/internal.h"
//#include "node_buffer.h"
//using namespace node;
using namespace v8;
using namespace Nan;
// hack to avoid conflict between 'node.h' namespace and
// 'node' declared inside internal.h
#define node node_eth

#undef node
#define node node
#define ETCHASH_EPOCH_LENGTH 60000U
#define ETHASH_EPOCH_LENGTH 30000U
#define ETCHASH_EPOCH_HEIGHT 11700000U  //11700000  2520000
// ethash_light_new(block_number)
// returns: { block_number: Number, cache: Buffer }
NAN_METHOD(ethash_light_new) {
  Nan::HandleScope scope;

  // get block number argument
  v8::Local<v8::Object> block_number_v8 = info[0].As<v8::Object>();
  CHECK_TYPE_NUMBER(block_number_v8, BLOCKNUM_TYPE_INVALID);
  // node -> C
  const uint64_t block_number = block_number_v8->IntegerValue();

  // get new ethash_light handler
  const int epoch_length = block_number >= ETCHASH_EPOCH_HEIGHT ? ETCHASH_EPOCH_LENGTH : ETHASH_EPOCH_LENGTH;
  const int epoch  = block_number / ETHASH_EPOCH_LENGTH;
  const int epoch2 = block_number / (block_number >= ETCHASH_EPOCH_HEIGHT ? ETCHASH_EPOCH_LENGTH : ETHASH_EPOCH_LENGTH);
  ethash_light_t light = ethash_light_new(block_number,epoch,epoch2);
  if (light == NULL) {
    return Nan::ThrowError(LIGHTNEW_NOMEM);
  }

  // C -> node
  v8::Local<v8::Object> obj = Nan::New<v8::Object>();
  obj->Set(Nan::New<v8::String>("block_number").ToLocalChecked(),
    Nan::New<v8::Number>(block_number));
  obj->Set(Nan::New<v8::String>("cache").ToLocalChecked(),
    COPY_BUFFER((const char *)light->cache, light->cache_size));
  info.GetReturnValue().Set(obj);

  // free ethash_light handler
  ethash_light_delete(light);
}

// ethash_light_compute(block_number, cache, header_hash, nonce)
// returns: { mix_hash: Buffer, result: Buffer }
NAN_METHOD(ethash_light_compute) {
  Nan::HandleScope scope;

  struct ethash_light light;

  // get block number argument
  v8::Local<v8::Object> block_number_v8 = info[0].As<v8::Object>();
  CHECK_TYPE_NUMBER(block_number_v8, BLOCKNUM_TYPE_INVALID);
  // node -> C
  light.block_number = block_number_v8->IntegerValue();

  // get cache argument
  v8::Local<v8::Object> cache_v8 = info[1].As<v8::Object>();
  CHECK_TYPE_BUFFER(cache_v8, CACHE_TYPE_INVALID);
  // node -> C
  light.cache = (void *) node::Buffer::Data(cache_v8);
  light.cache_size = node::Buffer::Length(cache_v8);

  // get header hash
  v8::Local<v8::Object> header_hash_v8 = info[2].As<v8::Object>();
  CHECK_TYPE_BUFFER(header_hash_v8, HEADERHASH_TYPE_INVALID);
  CHECK_BUFFER_LENGTH(header_hash_v8, 32, HEADERHASH_LENGTH_INVALID);
  // node -> C
  ethash_h256_t *header_hash = (ethash_h256_t *) node::Buffer::Data(header_hash_v8);

  // get nonce argument
  v8::Local<v8::Object> nonce_v8 = info[3].As<v8::Object>();
  CHECK_TYPE_BUFFER(nonce_v8, NONCE_TYPE_INVALID);
  CHECK_BUFFER_LENGTH(nonce_v8, 8, NONCE_LENGTH_INVALID);
  // node -> C
  const uint64_t nonce = be64toh(*((uint64_t *) node::Buffer::Data(nonce_v8)));

  // calculate light client data
  ethash_return_value_t ret = ethash_light_compute(&light, *header_hash, nonce);
  if (!ret.success) {
    return Nan::ThrowError(LIGHTCOMPUTE_ERROR);
  }

  // C -> node
  v8::Local<v8::Object> obj = Nan::New<v8::Object>();
  obj->Set(Nan::New<v8::String>("mix_hash").ToLocalChecked(),
    COPY_BUFFER((const char *)&ret.mix_hash, sizeof(ethash_h256_t)));
  obj->Set(Nan::New<v8::String>("result").ToLocalChecked(),
    COPY_BUFFER((const char *)&ret.result, sizeof(ethash_h256_t)));
  info.GetReturnValue().Set(obj);
}

// ethash_light_new_internal(cache_size, seed)
// returns: Buffer
NAN_METHOD(ethash_light_new_internal) {
  Nan::HandleScope scope;

  // get cache size
  v8::Local<v8::Object> cache_size_v8 = info[0].As<v8::Object>();
  CHECK_TYPE_NUMBER(cache_size_v8, CACHESIZE_TYPE_INVALID);
  // node -> C
  const uint64_t cache_size = cache_size_v8->IntegerValue();

  // get seed
  v8::Local<v8::Object> seed_v8 = info[1].As<v8::Object>();
  CHECK_TYPE_BUFFER(seed_v8, SEED_TYPE_INVALID);
  CHECK_BUFFER_LENGTH(seed_v8, 32, SEED_LENGTH_INVALID);
  // node -> C
  ethash_h256_t *seed = (ethash_h256_t *) node::Buffer::Data(seed_v8);

  // Frustrated attempt on optimizing buffer creation to avoid copying =/
  // ethash_compute_cache_nodes() is static and can't be used inside _light_new_internal()
  // left here to aid future attempts {
  //
  // // get a buffer for cache
  // v8::Local<v8::Object> buf = Nan::NewBuffer(cache_size).ToLocalChecked();
  // // local light structure
  // struct ethash_light light;
  //
  // // get new cache
  // _light_new_internal(node::Buffer::Data(buf), &light, cache_size, seed);
  //
  // // C -> node
  // info.GetReturnValue().Set(buf);
  //
  // }

  // let there be light
  ethash_light_t light = ethash_light_new_internal(cache_size, seed);
  if (light == NULL) {
    return Nan::ThrowError(LIGHTNEW_NOMEM);
  }

  // C -> node
  info.GetReturnValue().Set(COPY_BUFFER((const char *)light->cache, light->cache_size));

  // free ethash_light handler
  ethash_light_delete(light);
}

// ethash_light_compute_internal(cache, full_size, header_hash, nonce)
// returns: { mix_hash: Buffer, result: Buffer }
NAN_METHOD(ethash_light_compute_internal) {
  Nan::HandleScope scope;

  struct ethash_light light;

  // get cache argument
  v8::Local<v8::Object> cache_v8 = info[0].As<v8::Object>();
  CHECK_TYPE_BUFFER(cache_v8, CACHE_TYPE_INVALID);
  // node -> C
  light.cache = (void *) node::Buffer::Data(cache_v8);
  light.cache_size = node::Buffer::Length(cache_v8);

  // get full_size argument
  v8::Local<v8::Object> full_size_v8 = info[1].As<v8::Object>();
  CHECK_TYPE_NUMBER(full_size_v8, FULLSIZE_TYPE_INVALID);
  // node -> C
  uint64_t full_size = full_size_v8->IntegerValue();

  // get header hash
  v8::Local<v8::Object> header_hash_v8 = info[2].As<v8::Object>();
  CHECK_TYPE_BUFFER(header_hash_v8, HEADERHASH_TYPE_INVALID);
  CHECK_BUFFER_LENGTH(header_hash_v8, 32, HEADERHASH_LENGTH_INVALID);
  // node -> C
  ethash_h256_t *header_hash = (ethash_h256_t *) node::Buffer::Data(header_hash_v8);

  // get nonce argument
  v8::Local<v8::Object> nonce_v8 = info[3].As<v8::Object>();
  CHECK_TYPE_BUFFER(nonce_v8, NONCE_TYPE_INVALID);
  CHECK_BUFFER_LENGTH(nonce_v8, 8, NONCE_LENGTH_INVALID);
  // node -> C
  const uint64_t nonce = be64toh(*((uint64_t *) node::Buffer::Data(nonce_v8)));

  ethash_return_value_t ret = ethash_light_compute_internal(
    &light, full_size, *header_hash, nonce);
  if (!ret.success) {
    return Nan::ThrowError(LIGHTCOMPUTE_ERROR);
  }

  // C -> node
  v8::Local<v8::Object> obj = Nan::New<v8::Object>();
  obj->Set(Nan::New<v8::String>("mix_hash").ToLocalChecked(),
    COPY_BUFFER((const char *)&ret.mix_hash, sizeof(ethash_h256_t)));
  obj->Set(Nan::New<v8::String>("result").ToLocalChecked(),
    COPY_BUFFER((const char *)&ret.result, sizeof(ethash_h256_t)));
  info.GetReturnValue().Set(obj);
}

#define THROW_ERROR_EXCEPTION(x) Nan::ThrowError(x)
//#include <node_buffer.h>

NAN_METHOD(etchash) {
	if (info.Length() != 3) return THROW_ERROR_EXCEPTION("You must provide 3 arguments: header hash (32 bytes), nonce (8 bytes), height (integer)");

	v8::Isolate *isolate = v8::Isolate::GetCurrent();

	Local<Object> header_hash_buff = info[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
	if (!Buffer::HasInstance(header_hash_buff)) return THROW_ERROR_EXCEPTION("Argument 1 should be a buffer object.");
	if (Buffer::Length(header_hash_buff) != 32) return THROW_ERROR_EXCEPTION("Argument 1 should be a 32 bytes long buffer object.");

	Local<Object> nonce_buff = info[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
	if (!Buffer::HasInstance(nonce_buff)) return THROW_ERROR_EXCEPTION("Argument 2 should be a buffer object.");
	if (Buffer::Length(nonce_buff) != 8) return THROW_ERROR_EXCEPTION("Argument 2 should be a 8 bytes long buffer object.");

        if (!info[2]->IsNumber()) return THROW_ERROR_EXCEPTION("Argument 3 should be a number");
        const int height = Nan::To<int>(info[2]).FromMaybe(0);

	ethash_h256_t header_hash;
	memcpy(&header_hash, reinterpret_cast<const uint8_t*>(Buffer::Data(header_hash_buff)), sizeof(header_hash));
        const uint64_t nonce = __builtin_bswap64(*(reinterpret_cast<const uint64_t*>(Buffer::Data(nonce_buff))));

        static int prev_epoch_seed = 0;
        static ethash_light_t cache = nullptr;
        const int epoch_length = height >= ETCHASH_EPOCH_HEIGHT ? ETCHASH_EPOCH_LENGTH : ETHASH_EPOCH_LENGTH;
        const int epoch       = height / epoch_length;
        const int epoch_seed  = (epoch * epoch_length + 1) / ETHASH_EPOCH_LENGTH;
        if (prev_epoch_seed != epoch_seed) {
            if (cache) ethash_light_delete(cache);
            cache = ethash_light_new(height, epoch_seed, epoch);
            prev_epoch_seed = epoch_seed;
        }
        ethash_return_value_t res = ethash_light_compute(cache, header_hash, nonce);

        v8::Local<v8::Array> returnValue = New<v8::Array>(2);
        Nan::Set(returnValue, 0, Nan::CopyBuffer((char*)&res.result.b[0], 32).ToLocalChecked());
        Nan::Set(returnValue, 1, Nan::CopyBuffer((char*)&res.mix_hash.b[0], 32).ToLocalChecked());
	info.GetReturnValue().Set(returnValue);
}


NAN_MODULE_INIT(Init) {
  Nan::Export(target, "ethash_light_new", ethash_light_new);
  Nan::Export(target, "ethash_light_compute", ethash_light_compute);
  Nan::Export(target, "ethash_light_new_internal", ethash_light_new_internal);
  Nan::Export(target, "ethash_light_compute_internal", ethash_light_compute_internal);
//  Nan::Export(target, "etchash", etchash);
  Nan::Set(target, Nan::New("etchash").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(etchash)).ToLocalChecked());
}

NODE_MODULE(ethash, Init);
