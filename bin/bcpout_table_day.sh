. $HOME/.bashrc

USER_EXB=tcop
DBPASSWD=tcoppw
DATABASE=$1

DATE=`date '+%Y%m%d'`
DB_EXP=`echo "cbcp_${DATE}"`

BACKUP_IP="10.9.128.92"
FTPUSER=szfh
FTPPASSWD=szfh

if [ -z "$1" ]; then
    echo "Usage: $0 dbname"
    echo ""
    exit -1
fi

rm -rf ${HOME}/bcpday/${DB_EXP}  2>/dev/null
mkdir ${HOME}/bcpday 2>/dev/null
mkdir ${HOME}/bcpday/${DB_EXP} 2>/dev/null

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
#cd db.bak/backup
#        mkdir ${DB_EXP}
#        cd ${DB_EXP}
        cd tcqs/tcop
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

cd ${HOME}/bcpday; _remove_oldbak
cd ${HOME}/bcpday/${DB_EXP}; 
echo "syspara trnjour acctjour bankinfo operinfo ebanksumm baginfo agreement notetypemap errinfo errmap organinfo noteinfo funclist reconinfo feesum feelist svcclass freemsg queryinfo generalcode colsdesc feetype regioninfo">>tables.tmp

for table in `cat tables.tmp`
do
bcp $DATABASE..$table out $DATABASE..$table -U$USER_EXB -P$DBPASSWD -S$DSQUERY -c 1>/dev/null 2>/dev/null
echo "*** $table done ***"
done
echo -e "dump transaction $1 with no_log\ngo" >dump.sql
isql  -Usa -P -idump.sql
