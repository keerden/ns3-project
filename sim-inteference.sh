

output=inteference.$(date +%m%d%Y_%H%M%S)
maxa=16
maxb=16
echo "Running simulation:";
for ((a=1; a <= maxa ; a++))
do
 for ((b=1; b <= maxb ; b++))
 do
	echo "a=$a b=$b"
	./waf --run "working3 --nStasB=$b  --nStasA=$a --raw" >> $output

 done
done
echo "Done!";