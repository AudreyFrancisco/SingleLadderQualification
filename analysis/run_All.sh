#!/bin/bash
# Run run_FullTuneCal over a range of inital charges:
for i in {3..7}  #8 to 128
do
  bash run_FullTuneCal.sh $((2**$i))
done
