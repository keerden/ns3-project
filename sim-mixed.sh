

output=inteference-mix.$(date +%m%d%Y_%H%M%S)
maxa=16
maxb=16
echo "Running simulation:";
for ((a=1; a <= maxa ; a++))
do
 for ((b=1; b <= maxb ; b++))
 do
	echo "a=$a b=$b"
	./waf --run "interference --nStasB=$b  --nStasA=$a --mixed --raw" >> $output

 done
done
echo "Done!";
