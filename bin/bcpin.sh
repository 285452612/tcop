. $HOME/.bashrc

USER_EXB=tcop
DBPASSWD=tcoppw
DATABASE=$1

DATE=`date '+%Y%m%d'`
DB_EXP=`echo "cbcp_${DATE}"`

mkdir ${HOME}/db.bak 2>/dev/null
mkdir ${HOME}/db.bak/${DB_EXP} 2>/dev/null


DATE=`date '+%Y%m%d'`
echo -e "\n请输入要导入数据的日期(格式: YYYYMMDD 如 $DATE): \c"
read restore_date
DB_EXP=`echo -e "cbcp_${restore_date}"`
DBFILE=`echo "${restore_date}_$1.tar"`


if [ -f ${HOME}/db.bak/${DB_EXP}/${DBFILE}.gz ]
then
	cd ${HOME}/db.bak/${DB_EXP}; 
	gzip -d ${DBFILE}.gz;
	tar xvf ${DBFILE} 1>/dev/null;
	
	echo "            现在开始导入$1数据..."
	echo "use $DATABASE" >a.sql
	echo "go" >>a.sql
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

	echo -e  "use $1\ngo" >dlt_tab.sql

	for table in `cat tables.tmp`
	do
#		echo -e "delete from $table\ngo" >>dlt_tab.sql
		echo -e "truncate table $table\ngo" >>dlt_tab.sql
	done

	isql -U$USER_EXB -P$DBPASSWD  -idlt_tab.sql

	cd ${HOME}/db.bak/${DB_EXP};
	for table in `cat tables.tmp`
	do
	bcp $DATABASE..$table in $DATABASE..$table -U$USER_EXB -P$DBPASSWD -S$DSQUERY -c 1>/dev/null 2>/dev/null
	echo "*** $table done ***"
	done
	echo -e "dump transaction $1 with no_log\ngo" >dump.sql
	isql  -Usa -P -idump.sql

	cd ${HOME}/db.bak/${DB_EXP}; rm ${DBFILE} 2>/dev/null ;
        tar cvf ${DBFILE} ${DATABASE}* 1>/dev/null 2>/dev/null; sleep 5; sync;
        rm ${DATABASE}*; rm *.sql; rm tables.tmp;
        gzip -f ${DBFILE} 1>/dev/null;
   	echo "  $1  数据库恢复成功!"  
else
	echo "   $1  没有所选历史备份数据!"
fi
