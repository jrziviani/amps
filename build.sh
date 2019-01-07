#!/bin/sh

init() {
    local rebuild="$1"

    [[ -d bin ]] && rm -fr bin
    [[ -d .build && "$rebuild" == "--rebuild" ]] && rm -fr .build
}

build() {
    local rebuild="$1"

    echo "[Building]---------------------"
    echo ""
    [[ "$rebuild" == "--rebuild" ]] && mkdir .build
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
    local rebuild="$1"

    init "$rebuild"
    build "$rebuild"
    echo "[Build finished]"
    echo ""
    run_test_cases
    echo ""
}

main "$1"
