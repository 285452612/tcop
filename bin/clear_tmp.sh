#!/bin/bash

. $HOME/.bashrc

CURDATE=`date '+%Y%m%d'`

delete_file()
{
    if [ ! -d $1 ]; then
        return 1
    fi
    if [ ${#} -eq 2 ]; then
        find $1 -type f -mtime +$2 | xargs rm -rf >/dev/null 2>/dev/null
    elif [ ${#} -eq 3 ]; then
        find $1 -type f -name "$2" -mtime +$3 | xargs rm -rf >/dev/null 2>/dev/null
    else
        echo "delete_file path [ name ] mtime"
        return 1
    fi
    find $1 -type f -size 0c -exec rm -rf {} \; >/dev/null 2>/dev/null
    return 0
}

delete_dir()
{
    find $1 -type d -mtime +$2 -prune | xargs rm -rf >/dev/null 2>/dev/null
    return 0
}

rename_file()
{
    if [ -f $1 ]; then
        cat $1 >> $1.$CURDATE
        rm -rf $1
    fi
}

dump_transaction()
{
    sql_file=`date '+%H%I%M'`".sql"
    echo -e "dump transaction opdb with no_log\ngo\n" > $sql_file
    isql -Usa -P -i $sql_file
    rm -f $sql_file
}

delete_file $FILES_DIR 'tmp*' 7
delete_file $FILES_DIR '[0-9]*.*' 15 
delete_file $FILES_DIR '[a-z]*.*' 15 
delete_file $HOME/log 30 
delete_file $HOME/consoleprint 7 
delete_file $HOME/data 15 
delete_file $HOME/bankdata 2 
rename_file $HOME/log/comlog
rename_file $HOME/log/errlog

dump_transaction
