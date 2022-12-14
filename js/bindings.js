'use strict'

const ethUtil = require('ethereumjs-util');
const ethashjs = require('ethashjs');
const ethHashUtil = require('ethashjs/util');
const ethashcpp = require('bindings')('ethash');
const deasync = require('deasync');

var messages = require('./messages');

var Ethash = module.exports = function (cacheDB) {
    this.dbOpts = {
        valueEncoding: 'json'
    };
    this.cacheDB = cacheDB;
    this.cache = false;
    this.light = false;
}

// ethash_light_new(block_number)
// returns: { block_number: Number, cache: Buffer }
Ethash.prototype.ethash_light_new = ethashcpp.ethash_light_new;

// ethash_light_compute(light, header_hash, nonce)
// returns: { mix_hash: Buffer, result: Buffer }
Ethash.prototype.ethash_light_compute = function (light, header_hash, nonce) {
    if (!light || !light.hasOwnProperty('block_number') || !light.hasOwnProperty('cache')) {
        throw new TypeError(messages.LIGHT_OBJ_INVALID);
    }
    //console.log('headerhash:'+header_hash+','+header_hash.length+'.nonce:'+nonce)

    return ethashcpp.ethash_light_compute(light.block_number, header_hash, nonce);
}

// mkcache(cacheSize, seed)
// returns: arrays of cache lines
Ethash.prototype.mkcache = function (cacheSize, seed) {
    // get new cache from cpp
    this.cache = ethashcpp.ethash_light_new_internal(cacheSize, seed);
    // cache is a single Buffer here! Not an array of cache lines.
    return this.cache;
}

// run(val, nonce, fullSize)
// returns: { mix_hash: Buffer, result: buffer }
Ethash.prototype.run = function (val, nonce, fullSize) {
    // get new cache from cpp
    return ethashcpp.ethash_light_compute_internal(this.cache, fullSize, val, nonce);
}

// doHash(state, val, nonce)
// returns: { mix_hash: Buffer, result: buffer }
Ethash.prototype.doHash = function(val, nonce) {
    return ethashcpp.ethash_light_compute_internal(this.cache, this.fullSize, val, nonce);
}

// Ethash.prototype.doHash2 = function(val, nonce) {
//     return ethashcpp.ethash_light_compute(this.cache, this.fullSize, val, nonce);
// }

Ethash.prototype.headerHash = ethashjs.prototype.headerHash;

Ethash.prototype.cacheHash = function (state) {
    return ethUtil.sha3((state || this).cache);
}

// Get current epoch
Ethash.prototype.getEpoc = function(number) {
    return ethHashUtil.getEpoc(number);
}

Ethash.prototype.loadEpocSync = function (number) {
    let state = undefined;
    this.loadEpoc(number, (st) => {
        state = st;
    });
    deasync.loopWhile(() => { return !state; });
}

/**
 * Loads the seed and the cache given a block nnumber
 * @method loadEpoc
 * @param number Number
 * @param cb function
 */
Ethash.prototype.loadEpoc = function (number, cb) {
    const epoc = ethHashUtil.getEpoc(number);
    let self = this;

    if (this.epoc === epoc) {
        return cb();
    }

    this.epoc = epoc;
    // console.log("epoc",this.epoc)
    // gives the seed the first epoc found
    let findLastSeed = (ep, cb2) => {
        if (ep === 0) {
            return cb2(ethUtil.zeros(32), 0);
        }

        self.cacheDB.get(ep, self.dbOpts, function (err, data) {
            if (!err) {
                cb2(data.seed, ep);
            } else {
                findLastSeed(ep - 1, cb2);
            }
        });
    };

    let generate = (curr, begin) => {
        let [cacheSize, fullSize, seed] = [ethHashUtil.getCacheSize(epoc), ethHashUtil.getFullSize(epoc), ethHashUtil.getSeed(curr, begin, epoc)];
        return {
            cacheSize: cacheSize,
            fullSize: fullSize,
            seed: seed,
            cache: self.mkcache(cacheSize, seed)
        };
    };
    // console.log("self.cacheDB.get",epoc)
    /* eslint-disable handle-callback-err */
    self.cacheDB.get(epoc, self.dbOpts, (err, rec) => {

        let set = (r) => {
            console.log("r:",r)
            self.cache = r.cache;
            self.cacheSize = r.cacheSize;
            self.fullSize = r.fullSize;
            self.seed = new Buffer(r.seed);
            cb(self);
        };
        if (!rec) {
            return findLastSeed(epoc, (seed, begin) => {
                let rec = generate(seed, begin);
                // store the generated cache
                // console.log("rec",rec)
                self.cacheDB.put(epoc, rec, self.dbOpts, cb);
                set(rec);
            });
        }
        set(rec);
    });
    /* eslint-enable handle-callback-err */
}

/**
 * Loads the seed and the cache given epoc
 * @method loadNextEpoc
 * @param epoc Number
 * @param cb function
 */
Ethash.prototype.loadNextEpoc = function (epoc, cb) {
    // const epoc = ethHashUtil.getEpoc(number);
    let self = this;

    if (this.epoc === epoc) {
        return cb();
    }

    this.epoc = epoc;
    // console.log("epoc",this.epoc)
    // gives the seed the first epoc found
    let findLastSeed = (ep, cb2) => {
        if (ep === 0) {
            return cb2(ethUtil.zeros(32), 0);
        }

        self.cacheDB.get(ep, self.dbOpts, function (err, data) {
            if (!err) {
                cb2(data.seed, ep);
            } else {
                findLastSeed(ep - 1, cb2);
            }
        });
    };

    let generate = (curr, begin) => {
        let [cacheSize, fullSize, seed] = [ethHashUtil.getCacheSize(epoc), ethHashUtil.getFullSize(epoc), ethHashUtil.getSeed(curr, begin, epoc)];
        return {
            cacheSize: cacheSize,
            fullSize: fullSize,
            seed: seed,
            cache: self.mkcache(cacheSize, seed)
        };
    };
    // console.log("self.cacheDB.get",epoc)
    /* eslint-disable handle-callback-err */
    self.cacheDB.get(epoc, self.dbOpts, (err, rec) => {

        let set = (r) => {
            // console.log("r:",r)
            self.cache = r.cache;
            self.cacheSize = r.cacheSize;
            self.fullSize = r.fullSize;
            self.seed = new Buffer(r.seed);
            cb(self);
        };
        if (!rec) {
            return findLastSeed(epoc, (seed, begin) => {
                let rec = generate(seed, begin);
                // store the generated cache
                // console.log("rec",rec)
                self.cacheDB.put(epoc, rec, self.dbOpts, cb);
                set(rec);
            });
        }
        set(rec);
    });
    /* eslint-enable handle-callback-err */
}

// Ethash.prototype.loadEpoc2 = function (number, cb) {
//     var self = this
//     const epoc = ethHashUtil.getEpoc(number)
//     if (this.epoc === epoc) {
//         return cb()
//     }
//
//     this.epoc = epoc
//
//     // gives the seed the first epoc found
//     function findLastSeed (epoc, cb2) {
//         if (epoc === 0) {
//             return cb2(ethUtil.zeros(32), 0)
//         }
//
//         self.cacheDB.get(epoc, self.dbOpts, function (err, data) {
//             if (!err) {
//                 cb2(data.seed, epoc)
//             } else {
//                 findLastSeed(epoc - 1, cb2)
//             }
//         })
//     }
//
//     /* eslint-disable handle-callback-err */
//     self.cacheDB.get(epoc, self.dbOpts, function (err, data) {
//         if (!data) {
//             self.cacheSize = ethHashUtil.getCacheSize(epoc)
//             self.fullSize = ethHashUtil.getFullSize(epoc)
//
//             findLastSeed(epoc, function (seed, foundEpoc) {
//                 self.seed = ethHashUtil.getSeed(seed, foundEpoc, epoc)
//                 var cache = self.mkcache(self.cacheSize, self.seed)
//                 // store the generated cache
//                 self.cacheDB.put(epoc, {
//                     cacheSize: self.cacheSize,
//                     fullSize: self.fullSize,
//                     seed: self.seed,
//                     cache: cache
//                 }, self.dbOpts, cb)
//             })
//         } else {
//             // Object.assign(self, data)
//             // cache is a single Buffer here! Not an array of cache lines.
//             self.cache = data.cache
//             self.cacheSize = data.cacheSize
//             self.fullSize = data.fullSize
//             self.seed = new Buffer(data.seed)
//             cb()
//         }
//     })
//     /* eslint-enable handle-callback-err */
// }
