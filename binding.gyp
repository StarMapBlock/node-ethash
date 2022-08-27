{
  "targets": [{
    "target_name": "ethash",
    "sources": [
      "./src/ethash.cc",
      "./src/ethash-src/src/libethash/io.c",
      "./src/ethash-src/src/libethash/internal.c",
      "./src/ethash-src/src/libethash/sha3.c",
      "./src/ethash-src/src/libethash/keccakf800.c",
      "./src/ethash-src/src/libethash/keccak.cpp",
      "./src/ethash-src/src/libethash/sha3.cpp"
    ],
    "cflags_c": [
      "-std=gnu11      -fPIC -DNDEBUG -Ofast -fno-fast-math -w"
    ],
    "cflags_cc+": [
      "-fexceptions",
      "-std=c++11"
    ],
    "cflags_cc!": [
      "-fno-exceptions"
    ],
    "cflags_cc": [
     "-std=gnu++11 -s -fPIC -DNDEBUG -Ofast -fno-fast-math -fexceptions -fno-rtti -Wno-class-memaccess -w"
    ],
    "include_dirs": [
      "./src/ethash-src/src",
      "<!(node -e \"require('nan')\")"
    ],
    'cflags!': [ '-fexceptions' ]
  }]
}
