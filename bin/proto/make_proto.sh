for file in `ls` 
do
    if [ "${file##*.}" == "proto" ]
    then
        echo $file
        protoc --cpp_out=../../frmpub/protocc $file
        protoc -o ../lua/pb/${file%%.*}.pb $file
    fi
done

