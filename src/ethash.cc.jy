#include <stdint.h>
#include <node.h>
#include <nan.h>
#include <v8.h>
#include <stdexcept>
#include "endian.h"
#include "util.h"
#include "messages.h"
#include "libethash/ethash.h"
// hack to avoid conflict between 'node.h' namespace and
// 'node' declared inside internal.h
#define node node_eth
#include "libethash/internal.h"
#undef node
#define node node

NAN_METHOD(ethash) {
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

        static int prev_epoch = 0;
        static ethash_light_t cache = nullptr;
        const int epoch = height / ETHASH_EPOCH_LENGTH;
        if (prev_epoch != epoch) {
            if (cache) ethash_light_delete(cache);
            cache = ethash_light_new(height, epoch, epoch);
            prev_epoch = epoch;
        }
        ethash_return_value_t res = ethash_light_compute(cache, header_hash, nonce);

        v8::Local<v8::Array> returnValue = New<v8::Array>(2);
        Nan::Set(returnValue, 0, Nan::CopyBuffer((char*)&res.result.b[0], 32).ToLocalChecked());
        Nan::Set(returnValue, 1, Nan::CopyBuffer((char*)&res.mix_hash.b[0], 32).ToLocalChecked());
	info.GetReturnValue().Set(returnValue);
}

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

        static int prev_epoch = 0;
        static ethash_light_t cache = nullptr;
        const int epoch  = height / ETHASH_EPOCH_LENGTH;
        const int epoch2 = height / (height >= ETCHASH_EPOCH_HEIGHT ? ETCHASH_EPOCH_LENGTH : ETHASH_EPOCH_LENGTH);
        if (prev_epoch != epoch) {
            if (cache) ethash_light_delete(cache);
            cache = ethash_light_new(height, epoch, epoch2);
            prev_epoch = epoch;
        }
        ethash_return_value_t res = ethash_light_compute(cache, header_hash, nonce);

        v8::Local<v8::Array> returnValue = New<v8::Array>(2);
        Nan::Set(returnValue, 0, Nan::CopyBuffer((char*)&res.result.b[0], 32).ToLocalChecked());
        Nan::Set(returnValue, 1, Nan::CopyBuffer((char*)&res.mix_hash.b[0], 32).ToLocalChecked());
	info.GetReturnValue().Set(returnValue);
}

NAN_MODULE_INIT(Init) {
   Nan::Set(target, Nan::New("ethash").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(ethash)).ToLocalChecked());
   Nan::Set(target, Nan::New("etchash").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(etchash)).ToLocalChecked());
}

NODE_MODULE(etchash, Init);
