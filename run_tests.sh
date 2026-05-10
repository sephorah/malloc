#!/bin/bash
directory=$1

if [ ! -d "temp/" ]; then
    mkdir "temp/"
fi

ls $directory -laR > temp/expected
LD_PRELOAD=./libmalloc.so ls $directory -laR > temp/actual
diff temp/actual temp/expected