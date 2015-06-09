#!/bin/sh
#params
# 1- Compiled customized cnnclassify.c
# 2- any image
# 3- the ccv net you are going to convert
# 4- the deepbeliefsdk network file that is going to be created
# 5- the ccv labels file related to the ccv net passed as 3rd parameter
./ccv_to_deepbeliefsdk ../samples/dex.png ../samples/image-net-2012.sqlite3 mytest2.ntwk ../samples/image-net-2012.words
