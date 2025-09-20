#!/bin/bash

set -e

#如果没有build目录，创建build
if [! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pdw`/build/*

cd `pwd`/build &&
    cmake .. &&
    make

cd ..

#把头文件拷贝到 /usr/include/mymuduo
#so拷贝到 /usr/lib
if [ ! -d  /usr/include/mymuduo ]
    mkdir  /usr/include/mymuduo
fi

for header in `ls *.h`
do
    cp $header /usr/include/mymuduo
done

cp `pwd`/lib/libmymuduo.so /usr/lib