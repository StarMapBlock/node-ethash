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
      "-std=gnu11      -fPIC -DNDEBUG -Ofast -fno-fast-math -w",
      "-Wall",
      "-Wno-maybe-uninitialized",
      "-Wno-uninitialized",
      "-Wno-unused-function",
      "-Wextra"
    ],
    "cflags_cc+": [
      "-fexceptions",
      "-std=gnu++11 -s -fPIC -DNDEBUG -Ofast -fno-fast-math -fexceptions -fno-rtti -Wno-class-memaccess -w"
    ],
    "cflags_cc!": [
      "-fno-exceptions"
    ],
    "include_dirs": [
      "./src/ethash-src/src",
      "<!(node -e \"require('nan')\")"
    ]
  }]
}
