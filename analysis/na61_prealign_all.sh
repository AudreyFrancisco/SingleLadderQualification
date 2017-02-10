#!/bin/bash

./na61_prealign.sh ~/work/na61/data/NoiseOccupancy_161212_002426/ ~/work/na61/data/vdal/vd_3pt_185.root -b -q
./na61_prealign.sh ~/work/na61/data/NoiseOccupancy_161211_235125/ ~/work/na61/data/vdal/vd_3pt_183.root -b -q

hadd -f ~/work/na61/data/alignment_vd/prealignment_vd_cr2_combined.root ~/work/na61/data/NoiseOccupancy_161211_235125/results_cr2//prealignment_vd_cr2.root ~/work/na61/data/NoiseOccupancy_161212_002426/results_cr2//prealignment_vd_cr2.root
