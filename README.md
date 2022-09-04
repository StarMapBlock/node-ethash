# node-ethash Power By StarMap

[![js-standard-style](https://cdn.rawgit.com/feross/standard/master/badge.svg)](https://github.com/feross/standard)

Native node bindings for [Etchash cpp implementation](https://github.com/ethereum/ethash).
This library is experimental. **Use at your own risk**.
Currently working on **Linux only (Tested 20.04Ubuntu)**. Tested on node v10.24.1 and npm v6.14.12

For details on this project, please see the Ethereum wiki (https://github.com/ethereum/wiki/wiki/Ethash)
and ethereumjs/ethashjs (https://github.com/ethereumjs/ethashjs).

## Installation

```
$ git clone https://github.com/StarMapBlock/etchashjs.git
$ cd node-ethash/
$ git submodule init
$ git submodule update
$ npm install
```

## API

If you are using ETC Mordor to test your network, change ETCHASH_EPOCH_HEIGHT to 2520000
```
#define ETCHASH_EPOCH_HEIGHT 2520000U
```

- [`new Ethash([cacheDB])`](#newethashcachedb)
- [`ethash.verifyPOW(block, cb)`](#ethashverifypowblock-cb)
- [`ethash.mkcache(cacheSize, seed)`](#ethashmkcachecachesize-seed)
- [`ethash.run(val, nonce, fullSize)`](#ethashrunval-nonce-fullsize)
- [`ethash.loadEpoc(number, cb)`](#ethashloadepocnumber-cb)

### `new Ethash([cacheDB])`
Creates a new instance of `Ethash`.

**Parameters**
- `cacheDB` - an instance of a `levelup` DB which is used to store the cache(s).
Need by [`ethash.verifyPOW()`](#ethashverifypowblock-cb) and
[`ethash.loadEpoc()`](#ethashloadepocnumber-cb)

### `ethash.verifyPOW(block, cb)`
Verifies the POW on a block and its uncles.

Note: uses [`ethash.loadEpoc()`](#ethashloadepocnumber-cb) to load cache.

**Parameters**  
- `block` - the [block](https://github.com/ethereum/ethereumjs-block) to verify
- `cb` - callback which is given a `Boolean` determining the validity of the block

### `ethash.mkcache(cacheSize, seed)`
Creates a cache.

NOTE: this is automatically done for in
[`ethash.verifyPOW()`](#ethashverifypowblock-cb)
so you do not need to use this function if you are just validating blocks.

**Parameters**
- `cacheSize` - the size of the cache
- `seed` - the seed as a `Buffer`

### `ethash.run(val, nonce, fullSize)`
Runs ethash on a given val/nonce pair.

NOTE: you need to run [`ethash.mkcache()`](#ethashmkcachecachesize-seed)
first before using this function.

**Parameters**
- `val` - header hash as `Buffer`
- `seed` - the seed as a `Buffer`
- `fullSize` - the fullsize of the cache

**Return**
an `Object` containing
- `hash`  - the hash of the value
- `mix` - the mix result

### `ethash.loadEpoc(number, cb)`
Loads block number epoc's cache from DB.

**Parameters**  
- `number` - the [block's](https://github.com/ethereum/ethereumjs-block) number
- `cb` - callback called after the epoc was loaded

## Test
`$ npm test`

## Performance (node-ethash vs ethashjs):
```
$ node benchmark/benchmark.js
Create 1 fixtures
++++++++++++++++++++++++++++++++++++++++++++++++++
Benchmarking: mkcache
--------------------------------------------------
bindings x 1.47 ops/sec ±2.31% (8 runs sampled)
ethashjs x 0.17 ops/sec ±13.59% (5 runs sampled)
==================================================
Benchmarking: run
--------------------------------------------------
bindings x 571 ops/sec ±3.52% (79 runs sampled)
ethashjs x 17.82 ops/sec ±1.58% (48 runs sampled)
==================================================
```


## TODO:
- Implement tests for:
  - [`ethash.verifyPOW()`](#ethashverifypowblock-cb)
  - [`ethash.loadEpoc()`](#ethashloadepocnumber-cb)
- Create more fixtures for [`ethash.run()`](#ethashrunval-nonce-fullsize)'s tests


## LICENSE

This library is free and open-source software released under the MIT license.
