#!/bin/sh

set -x

for x in bcm11130*.h bcm11351*.h bcm281*.h; 
do 
	echo $x >fn
	sed -i -e 's/\.h//' fn 
	var1=`cat fn`
	printf "#define CONFIG_IDENT_STRING \" - $var1\"\n" >> $x
	rm fn
done
