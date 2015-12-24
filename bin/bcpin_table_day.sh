. $HOME/.bashrc

USER_EXB=tcop
DBPASSWD=tcoppw
DATABASE=$1

DATE=`date '+%Y%m%d'`
DB_EXP=`echo "cbcp_${DATE}"`

mkdir ${HOME}/bcpday 2>/dev/null
mkdir ${HOME}/bcpday/${DB_EXP} 2>/dev/null


DATE=`date '+%Y%m%d'`
echo -e "\n请输入要导入数据的日期(格式: YYYYMMDD 如 $DATE): \c"
read restore_date
DB_EXP=`echo -e "cbcp_${restore_date}"`


cd ${HOME}/bcpday/${DB_EXP}; 

echo "syspara trnjour acctjour bankinfo operinfo ebanksumm baginfo agreement notetypemap errinfo errmap organinfo noteinfo funclist reconinfo feesum feelist svcclass freemsg queryinfo generalcode colsdesc feetype regioninfo">>tables1.tmp
echo "            现在开始导入$1数据..."

echo -e  "use $1\ngo" >dlt_tab.sql

for table in `cat tables1.tmp`
do
echo -e "truncate table $table\ngo" >>dlt_tab.sql
done

isql -U$USER_EXB -P$DBPASSWD  -idlt_tab.sql

cd ${HOME}/bcpday/${DB_EXP};
for table in `cat tables1.tmp`
do
bcp $DATABASE..$table in $DATABASE..$table -U$USER_EXB -P$DBPASSWD -S$DSQUERY -c 1>/dev/null 2>/dev/null
echo "*** $table done ***"
done
echo -e "dump transaction $1 with no_log\ngo" >dump.sql
isql  -Usa -P -idump.sql
echo "  $1  数据库恢复成功!"  
