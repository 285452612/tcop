. $HOME/.bashrc

USER_EXB=tcop
DBPASSWD=tcoppw
DATABASE=$1
TABLE=$2

DATE=`date '+%Y%m%d'`
DB_EXP=`echo "$2_${DATE}"`
DBFILE=`echo "${DATE}_$TABLE.tar"`
DELETEDATE=`date -d "-$3 days" '+%Y%m%d'`

BACKUP_IP="127.0.0.1"
FTPUSER=tcqs
FTPPASSWD=tcqs123

if [ -z "$1" ]; then
    echo "Usage: $0 dbname"
    echo ""
    exit -1
fi

if [ -z "$2" ]; then
    echo "Usage: $0 table"
    echo ""
    exit -1
fi

if [ -z "$3" ]; then
    echo "Usage: $0 days"
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
                echo "与远程主机通信失败 !\n"
                return 1
        fi
        echo "ftp 送 [ $1 ] 备份..."
        echo "user $2 $3
        bin
    	lcd ${HOME}/db.bak/${DB_EXP}
	    put ${DBFILE}.gz
        bye
        " | ftp -v -n -i $1
        echo "FTP $1 成功 !\n"
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

echo "            现在开始导出$TABLE数据..."

cd ${HOME}/db.bak; 
cd ${HOME}/db.bak/${DB_EXP}; 

bcp $DATABASE..$TABLE out $DATABASE..$TABLE -U$USER_EXB -P$DBPASSWD -S$DSQUERY -c 1>/dev/null 2>/dev/null
echo "            导出$TABLE 数据成功    "

if [ -f ${HOME}/db.bak/${DB_EXP}/$DATABASE..$TABLE ]
then
    cd ${HOME}/db.bak/${DB_EXP};
    echo -e "dump transaction $1 with no_log\ngo" >dump.sql
    isql  -Usa -P -idump.sql
    cd ${HOME}/db.bak/${DB_EXP}; rm ${DBFILE} 2>/dev/null ;
    tar cvf ${DBFILE} ${DATABASE}* 1>/dev/null; sleep 5; sync;
    rm ${DATABASE}*; rm *.sql; rm tables.tmp;
    gzip -f ${DBFILE};
    #  backup_ftp ${BACKUP_IP} ${FTPUSER} ${FTPPASSWD};
    echo "  $TABLE备份成功!"  
else
    echo "   $TABLE 备份失败!"	  
fi

echo "  开始删除$TABLE ${DELETEDATE}之前的数据..."  
echo "*** delete $TABLE before ${DELETEDATE}***"
echo "use $DATABASE" >a.sql
echo "go" >>a.sql
echo "delete from $TABLE where nodeid=10 and workdate < '${DELETEDATE}'" >>a.sql
#echo "delete from $TABLE where  workdate < '${DELETEDATE}'" >>a.sql
echo "go" >>a.sql
echo "exit" >>a.sql

isql -U$USER_EXB -P$DBPASSWD -ia.sql
rm *.sql
echo "*** 删除$TABLE ${DELETEDATE}之前的数据成功***"
