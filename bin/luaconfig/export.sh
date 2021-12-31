for file in `ls cfg/` 
do
    if [ "${file##*.}" == "xls" ]
    then
        #echo $file
        ./export_tb.out -x cfg/$file -l ../lua/config/${file%%.*}.lua -j ../lua/config/${file%%.*}.json
    fi
done

