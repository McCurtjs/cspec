#!/bin/bash

build_target="gcc"
build_vers=""
unit_test=false
build_type="Debug"
skip_cmake=false
args=""
read_args=true

while [ "$read_args" == true ] && [ "$1" != "" ]; do
  case "$1" in
    -h | --help)
      echo ": - --       Options"
      echo ": h help                                 : prints this message"
      echo ": t target   [gcc|clang|wasm|mingw|msvc] : sets build target"
      echo ": c standard [all|23|11|99]              : set C standard version"
      echo ": r release                              : release build (default is debug)"
      echo ": s skip-cmake                           : skips cmake"
      echo ": -- <args>                              : passes remaining args to built exe (if any)"
      exit
      ;;
    -- )
      args="$@"
      read_args=false
      ;;
    -t | --target)
      build_target="$2"
      shift 1
      ;;
    -c | --standard)
      if [ "$2" == "all" ]; then
        build_vers="23 11 99 "
      else
        build_vers+="$2 "
      fi
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

if [ "$build_vers" == "" ]; then build_vers="23"; fi

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


sources_test="
  cspec.c
  tst/test_main.c
  tst/cspec_spec.c
  tst/expr_spec.c
  tst/matcher_spec.c
  tst/container_spec.c
  tst/libcs_spec.c
  tst/out_spec.c
  tst/log_spec.c
  tst/mem_spec.c
"

# Run build based on target type

# Clang build targets
if [ "$build_target" = "wasm" ] || [ "$build_target" = "clang" ]; then

  flags_common="
    -Wall -Wextra -pedantic -Wno-gnu-zero-variadic-macro-arguments
    -I ./
  "

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

    flags_clang=" -std=c2x\
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

  flags_common="
    -Wall -Werror -Wno-unused-value
    -I ./
  "
  success="0"

  echo "$build_vers"

  if [[ "$build_vers" =~ "23" ]]; then
    gcc -std=c2x -o build/gcc/test.exe $sources_test $flags_common -pedantic
    success="$?"
    if [ "$success" == "0" ]; then
      echo "C23"
      ./build/gcc/test.exe $args
      success="$?"
    fi
  fi

  if [[ "$success" == "0" && "$build_vers" =~ "11" ]]; then
    gcc -std=c11 -o build/gcc/test11.exe $sources_test $flags_common
    success="$?"
    if [ "$success" == "0" ]; then
      echo "C11"
      ./build/gcc/test11.exe $args
      success="$?"
    fi
  fi

  if [[ "$success" == "0" && "$build_vers" =~ "99" ]]; then
    gcc -std=c99 -o build/gcc/test99.exe $sources_test $flags_common
    success="$?"
    if [ "$success" == "0" ]; then
      echo "C99"
      ./build/gcc/test99.exe $args
      success="$?"
    fi
  fi


  exit

  #if [ "$success" ]; then echo "blah"; fi

  #if [ "$success" == "0" ] && ./build/gcc/test.exe $args && success="$?";
  #[ "$success" == "0" ] && echo "Hello!";
  #[ "$success" == "0" ] && success=`gcc -std=c11 -o build/gcc/test_11.exe $gcc_params`
  #[ "$success" == "0" ] && success=`./build/gcc/test_11.exe $args`



#  if [ "$?" == "0" ]; then
#    ./build/gcc/test.exe $args
#
#    if [ "$?" == "0" ]; then
#      gcc -std=c11 -o build/gcc/test_c11.exe cspec.c tst/cspec_spec.c tst/test_main.c -I ./
#
#      if [ "$?" == "0" ]; then
#        ./build/gcc/test.exe $args
#      fi
#    fi
#  fi

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
