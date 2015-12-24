. $HOME/.bashrc

USER_EXB=tcop
DBPASSWD=tcoppw
DATABASE=$1

DATE=`date '+%Y%m%d'`
DB_EXP=`echo "cbcp_${DATE}"`
DBFILE=`echo "${DATE}_$1.tar"`

BACKUP_IP="10.9.128.247"
FTPUSER=pfyh
FTPPASSWD=pfyh123

if [ -z "$1" ]; then
    echo "Usage: $0 dbname"
    echo ""
    exit -1
fi

rm -rf ${HOME}/db.bak/${DB_EXP}/${DBFILE}  2>/dev/null
mkdir ${HOME}/db.bak 2>/dev/null
mkdir ${HOME}/db.bak/${DB_EXP} 2>/dev/null

#ftp hostip user passwd
backup_ftp()
{
        ping -c 1 $1 >/dev/null
        if [ $? -ne 0 ]; then
                echo "��Զ������ͨ��ʧ�� !\n"
                return 1
        fi
        echo "ftp �� [ $1 ] ����..."
        echo "user $2 $3
        bin
#cd db.bak/backup
#        mkdir ${DB_EXP}
#        cd ${DB_EXP}
        cd tcqs/tcop
    	lcd ${HOME}/db.bak/${DB_EXP}
	    put ${DBFILE}.gz
        bye
        " | ftp -v -n -i $1
        echo "FTP $1 �ɹ� !\n"
        return 0
}

_remove_oldbak(){
    FILE_NUM=`ls -tr|wc -l`
    if [ $FILE_NUM -gt 15 ]
    then
        for dbbak_dir in `ls -tr`
        do
           rm -rf $dbbak_dir

           FILE_NUM=`expr $FILE_NUM - 1`
           if [ $FILE_NUM -le 15 ]
           then
                break
           fi
        done
    fi
}

echo "            ���ڿ�ʼ����$1����..."

cd ${HOME}/db.bak; _remove_oldbak
cd ${HOME}/db.bak/${DB_EXP}; 
echo "use $DATABASE" >a.sql
echo "go" >>a.sql
#echo "select name from sysobjects where type='U' and loginame='exbadm'  order by name" >>a.sql
echo "select name from sysobjects where type='U'  order by name" >>a.sql
echo "go" >>a.sql
echo "exit" >>a.sql

isql -U$USER_EXB -P$DBPASSWD -otables.tmp -ia.sql

sed "1,2 d" tables.tmp >tmp
mv tmp tables.tmp
sed "$ d" tables.tmp >tmp
mv tmp tables.tmp
sed "1,$ s/^ \{1,10\}//g" tables.tmp >tmp
mv tmp tables.tmp

for table in `cat tables.tmp`
do
bcp $DATABASE..$table out $DATABASE..$table -U$USER_EXB -P$DBPASSWD -S$DSQUERY -c 1>/dev/null 2>/dev/null
echo "*** $table done ***"
done

TABLE=`sed -n '1p' tables.tmp`
if [ -f ${HOME}/db.bak/${DB_EXP}/$DATABASE..$TABLE ]
then
    cd ${HOME}/db.bak/${DB_EXP};
    echo -e "dump transaction $1 with no_log\ngo" >dump.sql
    #echo -e "dump transaction cddb with no_log\ngo" >dump.sql
    #echo -e "dump transaction hddb with no_log\ngo" >>dump.sql
    #echo -e "dump transaction plogdb with no_log\ngo" >>dump.sql
    isql  -Usa -P -idump.sql
    cd ${HOME}/db.bak/${DB_EXP}; rm ${DBFILE} 2>/dev/null ;
    tar cvf ${DBFILE} ${DATABASE}* 1>/dev/null; sleep 5; sync;
    rm ${DATABASE}*; rm *.sql; rm tables.tmp;
    gzip -f ${DBFILE};
    echo " ��ر��ݿ�ʼ"  
    backup_ftp ${BACKUP_IP} ${FTPUSER} ${FTPPASSWD};
    echo "  $1  ���ݿⱸ�ݳɹ�!"  
else
    echo "   $1  ���ݿⱸ��ʧ��!"	  
fi
