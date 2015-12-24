#FILELIST='syspara errinfo errmap feeset feetype notetypemap operinfo organinfo regioninfo'
FILELIST='bankinfo'
for table in $FILELIST
do
bcp opdb..$table out backdir/$table -Utcop -Ptcoppw -c
echo "*** $table backup done ***"
done
