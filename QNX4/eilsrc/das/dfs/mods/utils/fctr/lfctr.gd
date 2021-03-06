#! /bin/sh
#__USAGE
#%C	[options]
#Counts Log Files
#	-d <dirname>
#	-r <rootname>
#	-E output with lables
#	-O output DAS logger/reader options

dirname=$PWD
rootname=log
outopt=d

while getopts d:r:EO opt $*
do
	case $opt in
		d) dirname=$OPTARG;;
		r) rootname=$OPTARG;;
		E) outopt=e;;
		O) outopt=o;;
		\?) echo; exec use $0;;
	esac
done

dirs=`/bin/find $dirname -logical -level 1 -prune -type d -name $rootname\[0-9\]\[0-9\]\[0-9\]\[0-9\] -print | /bin/sort`

# find first file
typeset -i low=-1 high=-1 i=0 lastd=0 curd=0 fpd=0 numdirs=0
numdirs=`echo $dirs |wc -w`

for j in $dirs
do
   let lastd=curd
   f=`/bin/basename $j`
   curd=${f##$rootname}
   let i=i+1
   if test $i -ne 1 -a $i -ne $numdirs
   then
       continue
   fi
   if test $i -eq 1
   then
       files=`/bin/find $j -logical -level 1 -prune -type f -name $rootname\[0-9\]\[0-9\]\[0-9\]\[0-9\] -print | /bin/sort`
       for k in $files
       do
           f=`/bin/basename $k`
           low=${f##$rootname}
           break
       done
   fi
   if test $i -eq $numdirs
   then
       files=`/bin/find $j -logical -level 1 -prune -type f -name $rootname\[0-9\]\[0-9\]\[0-9\]\[0-9\] -print | /bin/sort -r`
       for k in $files
       do
           f=`/bin/basename $k`
           high=${f##$rootname}
           break
       done
   fi
done

# get files per directory
#if test $numdirs -gt 1
#then
let fpd=curd-lastd
#fi

# output
case $outopt in
    d) echo $low $high $fpd $dirname $rootname;;
    e) echo First Log File Number: $low
       echo Last Log File Number: $high
       echo Files per Directory: $fpd
       echo Directiry: $dirname
       echo Rootname: $rootname;;
    o) printf "-L %s -N %s -F %s -d %s -r %s" $high $fpd $low $dirname $rootname;;
   \?) exit 1;;
esac
