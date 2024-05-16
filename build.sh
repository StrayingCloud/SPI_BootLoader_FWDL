#!/bin/bash
project_dir=$(cd "$(dirname "$0")";'pwd')
build_dir=$project_dir/build

make_project() {
    mkdir -p $build_dir
    if [ "$1" == "debug" ];then
        echo for debug
        mkdir -p $build_dir/debug/lib
        mkdir -p $build_dir/debug/bin
        cd $build_dir
        #-DTOOLCHAIN_PREFIX="/usr/bin/"
        cmake  -DEXECUTABLE_OUTPUT_PATH=$build_dir/debug/bin \
            -DLIBRARY_OUTPUT_PATH=$build_dir/debug/lib -DCMAKE_BUILD_TYPE="debug" \
            -DPROJECT_SOURCE_DIR=./ $project_dir
        make
    elif [ "$1" == "release" ];then
        #-DTOOLCHAIN_PREFIX="arm-linux-gnueabihf-"
        echo for release
        mkdir -p $build_dir/release/lib
        mkdir -p $build_dir/release/bin
        cd $build_dir
        if [ "$#" -ge 2 ]; then
            build_define=$2
        else
            build_define=normal
        fi

        cmake  -DEXECUTABLE_OUTPUT_PATH=$build_dir/release/bin \
            -DLIBRARY_OUTPUT_PATH=$build_dir/release/lib -DCMAKE_BUILD_TYPE="release" \
            -DTHE_BUILD_DEFINE=$build_define -DPROJECT_SOURCE_DIR=./ $project_dir

        echo "------- make ---------"
        make
        echo "-------- make end ---------"

        cd ..
        # ./package_deb.sh

    else
        echo "do nothing."
    fi
}

#-DTOOLCHAIN_PREFIX="/usr/local/cmake-3.5.0-Linux-i386/bin/" 
clean_project(){
    # make clean
    rm -rf $build_dir
    rm -rf artifact

}

if [ "$#" -lt 1 ] || [  "$1" == "-h" ]; then
    echo "Usage: bash build.sh [debug|release|clean]"
    exit 1
fi

if [ "$1" == "clean" ]; then
    clean_project
elif  [ "$1" == "release" ] || [ "$1" == "debug" ];then
    make_project "$1" "$2" "$3"
else
    make_project "$1" "$2"
fi

