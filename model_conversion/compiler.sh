#!/bin/sh
clang -L "../lib" -I "../lib" ../bin/cnnclassify.c -lccv `cat ../lib/.deps` -o ccv_to_deepbeliefsdk
