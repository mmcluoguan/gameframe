#!/bin/bash

getcount()
{
	echo $(ps x | egrep -w "^\s*$1" | grep -v grep | wc -l)
}

getpid()
{
	if [ ! -f "pid/$1.pid" ]; then
		return
	fi
	wait_pid=$(cat pid/$1.pid)
	echo $(ps ax | awk '{ print $1 }' | grep -e "^$wait_pid$")
}

waitpid()
{
	if [ ! -f "pid/$1.pid" ]; then
		return
	fi
	pid=$(cat pid/$1.pid)
	result=$(getpid $1)
	starttime=$(date +%s)
	count=1;
	while [ "$result" = "$pid" ]
	do
		sleep 0.1
		result=$(getpid $1)

		if [ "$(($count%3))" -eq 1 ]; then
			echo -en "."
		elif [ "$(($count%3))" -eq 2 ]; then
			echo -en "."
		else
			echo -en ".\b\b\b"
		fi

		((++count))
	done
	endtime=$(date +%s)
	costtime=$(($endtime-$starttime))
	if [ "$costtime" -gt 1 ]; then
		echo -en " \033[31m["$costtime"s]\033[0m\c"
	fi
}

#停止服务
dostop()
{
	for serv in $SERVERLIST
	do
		if [ ! -f "pid/$serv.pid" ]; then
			continue
		fi
        pid=$(cat pid/$serv.pid)
		echo -n -e "stoping \033[31m$serv\033[0m:$pid "
		if [ "$(getpid $serv)" != "" ]; then
			kill -3 $pid
			waitpid $serv
		fi
		echo ""
	done
}

#开始服务
dostart()
{
	for serv in $SERVERLIST
	do
        oldIFS=$IFS
     	IFS=_
     	arr=($serv)  
        IFS=$oldIFS
		echo -e "starting \033[31m$serv\033[0m ..."
		case $MODE in
		release)
			$(pwd)/${arr[0]} $serv $CFGNAME 1
			SUFFIX=""
		;;
		*)
			$(pwd)/${arr[0]}$SUFFIX $serv $CFGNAME 1
		;;
		esac
	done
}

#显示状态
dostatus()
{
	echo "STAT 	  PID     TIME    COMMAND    PRARM";
	for serv in $SERVERLIST
	do
		if [ ! -f "pid/$serv.pid" ]; then
			continue;
		fi
		pid=$(cat pid/$serv.pid)
        count=$(getcount $pid)
		if [ $count -eq 1 ];then
		 	state=$(ps x | egrep -w "^\s*$pid" | awk -F " " '{print $1,$4,$5,$6}' | grep -v grep)
			echo -e "\033[35m[running]\033[0m $state \t\t\c"
		 	echo ""
		else
            oldIFS=$IFS
            IFS=_
            arr=($serv)  
            IFS=$oldIFS
		 	echo -e "\033[31m[stopped]\033[0m $pid         $(pwd)/${arr[0]}$SUFFIX $serv \t\t\c"
		 	echo ""
		fi
	done
}

#release|debug模式
MODE="debug"
#操作服务器动作
OPERATOR=""
#读取单个服务
SERVER=""
#读取启动服务器
SERVERLIST=""
#程序后缀
SUFFIX=".out"
#配置文件
CFGNAME="gameframe.ini"

# 切换到命令所在路径
cd `dirname $0`

count=1
for arg in $@
do
	if [ "$count" == "$#" ] ; then
		OPERATOR=$arg
	else
		SERVERLIST+="$arg "
	fi
    count=$((count+1))
done

#启动的服务器列表,按先后顺序排列
if [ "$SERVERLIST" == "" ] ; then
	SERVERLIST="dbvisit_account_master dbvisit_game_master dbvisit_world_master world_master game_20 login_10 gate_30"
fi

echo -e "======================================== \033[35m`echo $MODE|tr 'a-z' 'A-Z'`\033[0m ========================================"
case $OPERATOR in
stop)
	dostop
;;
start)
	dostart
    sleep 0.1
	dostatus
;;
restart)
	dostop
	dostart
	sleep 0.1
	dostatus
;;
status)
	dostatus
;;
*)
	dostatus
;;
esac
echo -e "======================================== \033[35m`echo $MODE|tr 'a-z' 'A-Z'`\033[0m ========================================"

