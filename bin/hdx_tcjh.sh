#/home/tcop/bin/GenInNoteAddFile

MM=`date +%Y%m%d`
ftp -n 10.9.128.43 <<!
user huidan huidan
asc
lcd /home/tcop/other_file
put 8901_$MM\_DTCJH.txt 
bye
!
