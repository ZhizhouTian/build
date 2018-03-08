#!/bin/bash /etc/ikcommon
Include sys/hdd_control.sh

GV_SERVER_ALIVE=/tmp/.gv_server_alive

boot()
{
	__auto_register_gv >/dev/null 2>&1 &
}

__show_data()
{
	if res=$(__get_proc_gv) ;then
		local $res
		local mc=$machine_code
		local err=0
	else
		local err=3
	fi

	if [ "$(cat $GV_SERVER_ALIVE)" == "0" ] ;then
		local err=2
	fi

	reboot_time=$(($(date +%s)+$reboot_time))
	local data=$(json_output reboot_time:int mc:str genuine:int err:int)
	json_append __json_result__ data:json

	return 0
}

#写入版本号和序列号到/proc/gv
__set_proc_gv()
{
	echo $1 > /proc/genuine_verification
	sleep 0.3
}

#查看/proc/gv
__get_proc_gv()
{
	cat /proc/genuine_verification
}

__get_gv_status()
{
	local res genuine
	if res=$(__get_proc_gv) ;then
		local $res
	fi

	if [ "$genuine" = "1" ];then
		return 0
	else
		return 1
	fi
}

#自动激活正版
__auto_register_gv()
{
	local sn_ret
	local over_time=1540828800 #结束时间 2018-10-30 00:00:00
	local sn=$(hdd_get_sn $BOOTHDD)

	#如果验证失败, 那么继续往下
	if [ "$sn" ];then
		__set_proc_gv $VERSION,$sn
		if __get_gv_status ;then
			return 0
		fi
	fi

	while :;do
		if [ "$(date +%s)" -gt "$over_time" ]; then
			return 0
		fi

		sleep 30

		if ret=$(__get_proc_gv) ;then
			local $ret
		fi

		#向服务器发送gwid，版本号，机器码，获得序列号
		if ! sn_ret=$(curl -m 3 -s "http://dev.ikuai8.com:8087/gv?gwid=$GWID&version=$VERSION&mc=$machine_code") ;then
			echo 0 > $GV_SERVER_ALIVE
			continue
		fi
		echo 1 > $GV_SERVER_ALIVE

		if [ "$sn_ret" ]; then
			local $sn_ret

			__set_proc_gv $VERSION,$sn
			if __get_gv_status ;then
				#将新的SN写入到磁盘中
				hdd_set_sn $BOOTHDD $sn
			fi
			return 0
		fi
	done
}

show()
{
	Show __json_result__
}

save()
{
	local res

	#已经是激活成功的,无需再重新激活
	if __get_gv_status ;then
		return 0
	fi

	__set_proc_gv $VERSION,$sn
	if __get_gv_status ;then
		hdd_set_sn $BOOTHDD $sn >/dev/null 2>&1
		return 0
	else
		echo "error SN"
		return 1
	fi
}
