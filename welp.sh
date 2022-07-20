#rm -Rf $3
mkdir $3

for file in $2\/*
do
  echo $file	
  outfile=$(echo $file | sed 's/'$2'/'$3'/g')
  echo $outfile
  $1 $file $outfile
done
