#!/bin/bash

make clean-all && make -j9 && make clean-all && make -j9 lib && make clean-all && make test_fifo && make clean-all && make lib -j9 && make lib_analysis -j9 && cd GUI && qmake && make && cd ../MosaicSrc/powerboard && make clean && make && cd GUI && make clean && qmake && make
    cd - && echo -e "\n\nOK\n\n" && exit 0 || echo -e "\n\nBROKEN\n\n" || exit 1
