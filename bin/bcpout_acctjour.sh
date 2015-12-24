. $HOME/.bashrc

USER_EXB=tcop
DBPASSWD=tcoppw
DATABASE=$1

DATE=`date '+%Y%m%d'`
DB_EXP=`echo "acctjour_${DATE}"`
DBFILE=`echo "${DATE}_acctjour.tar"`

BACKUP_IP="127.0.0.1"
FTPUSER=tcqs
FTPPASSWD=tcqs123

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

echo "            现在开始导出$1数据..."

#cd ${HOME}/db.bak; _remove_oldbak
cd ${HOME}/db.bak/${DB_EXP}; 
echo "use $DATABASE" >a.sql
echo "go" >>a.sql
#echo "select name from sysobjects where type='U' and loginame='exbadm'  order by name" >>a.sql
echo "select name from sysobjects where type='U'  order by name" >>a.sql
echo "go" >>a.sql
echo "exit" >>a.sql

#isql -U$USER_EXB -P$DBPASSWD -otables.tmp -ia.sql

bcp $DATABASE..acctjour out $DATABASE..$table -U$USER_EXB -P$DBPASSWD -S$DSQUERY -c 1>/dev/null 2>/dev/null
echo "*** $table done ***"

if [ -f ${HOME}/db.bak/${DB_EXP}/$DATABASE..acctjour ]
then
    cd ${HOME}/db.bak/${DB_EXP};
    echo -e "dump transaction $1 with no_log\ngo" >dump.sql
    isql  -Usa -P -idump.sql
    cd ${HOME}/db.bak/${DB_EXP}; rm ${DBFILE} 2>/dev/null ;
    tar cvf ${DBFILE} ${DATABASE}* 1>/dev/null; sleep 5; sync;
    rm ${DATABASE}*; rm *.sql; rm tables.tmp;
    gzip -f ${DBFILE};
    #  backup_ftp ${BACKUP_IP} ${FTPUSER} ${FTPPASSWD};
    echo "  $1..acctjour备份成功!"  
else
    echo "   $1..acctjour备份失败!"	  
fi
