#!/bin/bash

make

if [ $? -ne 0 ]; then
    echo "EXIT WITH ERROR..."
    exit 1
fi

echo ---------------------------

cp to_fsdir/* ../fsdir/
../createfs -i ../fsdir -o ../student-distrib/filesys_img

if [ $? -ne 0 ]; then
    echo "EXIT WITH ERROR..."
    exit 1
fi

echo ===========================

cd ../student-distrib

make clean
make

if [ $? -ne 0 ]; then
    echo "EXIT WITH ERROR..."
    exit 1
fi

echo SUCCESS