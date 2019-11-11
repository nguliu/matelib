#!/bin/sh

#说明：
#  虽然使用了C++14编译选项，但是除了rpc部分用了少量C++14的特性外，其余均为C++11编写

set -x

SOURCE_DIR=`pwd`
BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-debug}
INSTALL_DIR=${INSTALL_DIR:-../install-${BUILD_TYPE}}
CXX=${CXX:-g++}

mkdir -p $BUILD_DIR/$BUILD_TYPE-cpp14 \
	&& cd $BUILD_DIR/$BUILD_TYPE-cpp14 \
	&& cmake \
		   -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
		   -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
		   -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		   $SOURCE_DIR \
	&& make $*