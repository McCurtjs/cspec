#!/bin/bash

build_target="wasm"
unit_test=false
build_type="Debug"
skip_cmake=false
args=""

while [ "$1" != "" ]; do
  case "$1" in
    -t | --target)
      build_target="$2"
      shift 1
      ;;
    -h | --help)
      echo ": - -- Options"
      echo ": h help                                 : prints this message"
      echo ": t target  [wasm|gcc|clang|mingw|msvc]  : sets build target"
      echo ": r release                              : release build (default is debug)"
      echo ": a args    \" \"                          : passes args to built exe (if any)"
      echo ": s skip-cmake                           : skips cmake"
      exit
      ;;
    -a | --args)
      args="$2"
      shift 1
      ;;
    -r | --release)
      build_type="Release"
      ;;
    -s | --skip-cmake)
      skip_cmake=true
      ;;
    *)
      echo ": Unknown parameter: $1"
      exit
      ;;
  esac
  shift 1
done

# Get make executable
make_exe="no_make"

which make &> /dev/null
if [ "$?" == "0" ]; then
  make_exe="make"
else
  which mingw32-make &> /dev/null
  if [ "$?" == "0" ]; then
    make_exe="mingw32-make"
  fi
fi

# executable checks
case "$build_target" in
  "wasm" )
    which clang &> /dev/null
    if [ "$?" != "0" ]; then
      echo ": build $build_target: clang compiler not found, exiting build."
      exit
    fi
    ;;&
  "mingw" | "msvc" )
    which cmake &> /dev/null
    if [ "$?" != "0" ]; then
      echo ": build $build_target: cmake not found, exiting build."
      exit
    fi
    ;;&
  "mingw" )
    if [ "$make_exe" == "no_make" ]; then
      echo ": build $build_target: make not found, exiting build."
      exit
    fi
    ;;
esac

# Run build based on target type

# Clang build targets
if [ "$build_target" = "wasm" ] || [ "$build_target" = "clang" ]; then

  flags_common="-Wall -Wextra -Wno-missing-braces -I ./"
  sources_test="cspec.c tst/cspec_spec.c tst/test_main.c"

  flags_debug_opt="-g -O0"
  if [ "$build_type" = "Release" ]; then
    flags_debug_opt="-Oz -flto"
  fi

  mkdir -p build/$build_target

  # WASM
  if [ "$build_target" = "wasm" ]; then

    # -nostdinc doesn't work because wasi doesn't ship with stddef for some reason
    # --target=wasm32 for non-wasi build. It works, but no standard lib is painful
    flags_wasm="--target=wasm32 -D__WASM__ \
      -Wl,--allow-undefined -Wl,--no-entry -Wl,--lto-O3 \
      --no-standard-libraries \
    "

    clang $flags_wasm -o build/wasm/test.wasm \
      $flags_common $flags_debug_opt $sources_test

    cp build/wasm/test.wasm web/test.wasm

  # Native
  elif [ "$build_target" = "clang" ]; then

    flags_clang=" \
      -Dmalloc=cspec_malloc -Drealloc=cspec_realloc \
      -Dcalloc=cspec_calloc -Dfree=cspec_free \
    ";

    clang $flags_clang -o build/clang/test.exe \
      $flags_common $flags_debug_opt $sources_test

    if [ "$?" == "0" ]; then
      ./build/clang/test.exe $args
    fi

  fi

# GCC
elif [ "$build_target" = "gcc" ]; then

  mkdir -p build/gcc

  gcc -o build/gcc/test.exe cspec.c tst/cspec_spec.c tst/test_main.c -I ./

  if [ "$?" == "0" ]; then
    ./build/gcc/test.exe $args
  fi

# CMake MinGW on Windows with GCC
elif [ "$build_target" = "mingw" ]; then

  if [ "$skip_cmake" != true ]; then
    cmake -G "MinGW Makefiles" -S . -B build/mingw \
      -DCMAKE_BUILD_TYPE=$build_type -DCMAKE_MEMTEST=ON
    if [ "$?" != "0" ]; then
      exit
    fi
  fi

  pushd . &> /dev/null
  cd build/mingw
  eval $make_exe
  if [ "$?" == "0" ]; then
    ./CSpec_specs.exe $args
  fi
  popd &> /dev/null

# CMake MSVC
elif [ "$build_target" = "msvc" ]; then

  cmake -G "Visual Studio 17 2022" -S . -B build/msvc
  if [ "$?" != "0" ]; then
    exit
  fi

# No matching build types
else

  echo ": Invalid build target: $build_target"

fi
