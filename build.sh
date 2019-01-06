#!/bin/sh

init() {
    [[ -d bin ]] && rm -fr bin
    [[ -d build ]] && rm -fr build
}

build() {
    echo "[Building]---------------------"
    echo ""
    mkdir .build
    cd .build
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
    [[ $? -eq 0 ]] || exit 1
    make
    mv bin ../bin
    cd ..
}

run_test_cases() {
    echo "[Running Tests]----------------"
    cp test/block* bin
    cd bin
    ./volt_test
    cd ..
}

main() {
    init
    build
    echo "[Build finished]"
    echo ""
    run_test_cases
    echo ""
}

main
