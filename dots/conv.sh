#!/bin/sh

i=0;
for f in *.dot
do
	out="$(printf '%04d' $i).png"
	echo "$f -> $out"
   	dot -Tpng "$f" > "$out"
   	i=$((i+1))
done

convert -resize 50% -dispose previous -delay 40 -loop 0 *.png out.gif
