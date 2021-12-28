#!/bin/bash
read_dir()
{
    for file in `ls $1` #注意此处这是两个反引号，表示运行系统命令
    do
        if [ -d $1"/"$file ]  
        then
            #在此处处理文件目录
            if [[ "$file" != "obj" && "$file" != "bin" && "$file" != "proto" ]]
            then
                ##echo $target/$1"/"$file
                mkdir -p $target/$1"/"$file
                read_dir $1"/"$file
            fi
        else
            #在此处处理文件
            if [ "${file##*.}" == "h" ]
            then
                #echo $1"/"$file
                /usr/bin/cp -f $1"/"$file $target/$1
            fi
        fi
    done
}
target=~/test
shynet="shynet"
frmpub="frmpub"

mkdir -p $target/$shynet
read_dir $shynet
echo "导出完成:"$target/$shynet
mkdir -p $target/$frmpub
read_dir $frmpub
echo "导出完成:"$target/$shynet
