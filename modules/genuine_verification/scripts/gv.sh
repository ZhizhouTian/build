#!/bin/bash /etc/ikcommon

IK_DB_CONFIG=/root/gv.db
TABLE_GV=sn

PROC_MC=/proc/mc
PROC_GV=/proc/gv

#写入版本号和序列号到/proc/gv
function set_proc_gv()
{
	local ret
	echo $1 > $PROC_GV
	ret = $?
	if [ "$?" != "0" ]; then
		#如果返回非0或者负数则返回jason={err:1}并直接退出程序
		echo "jason={err:1}" > /dev/tty
		exit 1
	fi
	return $ret
}

#查看/proc/gv
function get_proc_gv()
{
	#返回0-验证失败，1-验证成功,2-正在验证中
	local ret=$(cat $PROC_GV)
	if [ "$ret" == "1" ]; then
		return 1
	elif [ "$ret" == "0" ]; then
		return 0
	elif [ "ret" == "2" ]; then
		return 2
	else
		echo "jason={err:4}" > /dev/tty
		exit 1
	fi
}

function start_gv()
{
	local ret
	set_proc_gv $1,$2

	while true; do
		get_proc_gv
		ret=$?
		if [ "$ret" != "2" ]; then
			break;
		fi
		sleep 1
	done

	return $ret
}

function get_ma01_sn()
{
	#如果日期超过2018年3月1日，则返回jason={mc genuine:0}表示非正版
	local currtime=`date`
	local deadline_ma01="2018-03-01 00:00:00"
	local t1=`date -d "$currtime" +%s`
	local t2=`date -d "$deadline_ma01" +%s`

	if [ $t1 -gt $t2 ]; then
		echo "jason={mc:$(cat $PROC_MC)genuine:0}" > /dev/tty
		return 0
	fi

	echo "start get sn from server" > /dev/tty
	#向服务器发送版本号，gwid，机器码，获得序列号
		#如果序列号为0,表示服务器已关闭或者出错，返回jason={err:3}
		#如果序列号不为0
			#如果start_gv()返回1则返回jason={genuine:1}表示正版
				#将原有的数据库table清空，插入新的版本号和序列号
			#否则返回jason={err:3}表示序列号服务器出错
		#如果超时则返回jason={err:2}表示无法访问服务器
	return 0
}

#向页面展示当前是否正版
function show()
{
	#get_proc_gv()，如果返回1则返回jason={genuine:1}表示正版并退出
	get_proc_gv
	if [ "$?" == "1" ]; then
		echo "jason={genuine:1}" > /dev/tty
		return 1
	fi

	#从数据库中查询获得版本号和序列号
	sql_config_get_list $IK_DB_CONFIG "select * from $TABLE_GV" | if read config; then
	#如果有数据，start_gv()
		local $config
		#如果start_gv()返回1则返回jason={genuine:1}表示正版
		start_gv $version $sn
		if [ "$?" == "1" ]; then
			echo "jason={genuine:1}" > /dev/tty
			return 1
		else
			#否则调用get_ma01_sn
			get_ma01_sn
			return $?
		fi
	else
		#否则调用get_ma01_sn
		get_ma01_sn
		return $?
	fi
}

#页面传递来序列号
function check()
{
	local $S0

	#get_proc_gv()，如果返回1则返回jason={genuine:1}表示正版并退出
	get_proc_gv
	if [ "$?" == "1" ]; then
		return 1
	fi

	#通过/etc/release获得当前版本
	#如果start_gv()返回1则返回jason={genuine:1}表示正版
	start_gv $VERSION $sn
	if [ "$?" == "1" ]; then
		echo "jason={genuine:1}" > /dev/tty
		return 1
	else
	#否则返回jason={genuine:0}表示非正版
		echo "jason={mc:$(cat $PROC_MC)genuine:0}" > /dev/tty
		return 0
	fi
}
