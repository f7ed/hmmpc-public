#!/bin/bash



tar_name=hmmpc-public.tar.gz
eigen_name=env-setup/eigen-3.4.0.tar.gz
sodium_name=env-setup/libsodium-1.0.17-stable.tar.gz
cmake_name=env-setup/cmake-3.26.3.tar.gz
gcc_name=gcc
mpfr_name=env-setup/mpfr-4.2.0.tar.gz
gmp_name=env-setup/gmp-6.2.0.tar.bz2
mpc_name=env-setup/mpc-1.3.0.tar.gz                           

servers=(
ubuntu@34.92.79.41
ubuntu@35.241.123.3
ubuntu@34.92.97.28
ubuntu@35.241.90.108
ubuntu@34.92.160.188
ubuntu@34.96.225.243
ubuntu@35.220.167.23
ubuntu@34.96.214.210
ubuntu@34.92.195.217
ubuntu@35.241.90.184
ubuntu@34.92.60.130
)

private_key=/Users/fred/.ssh/xiang1

if [ "$1" == "hmmpc-public" ]
then
    make clean
    cd ..
    tar cvfz $tar_name --exclude=hmmpc-public/Exp-Data --exclude=hmmpc-public/Infer-Data --exclude=hmmpc-public/mnist --exclude=hmmpc-public/Player-Data --exclude=hmmpc-public/.git hmmpc-public
    remote_dir=/home/ubuntu/inferTest
    for((i=0;i<=10;i++)); do
        scp -i $private_key $tar_name ${servers[$i]}:$remote_dir
    done
    
fi

if [ "$1" == "cloud" ]
then
    remote_dir=/home/ubuntu/tools
    for((i=1;i<=10;i++)); do
        
        scp -i $private_key $eigen_name ${servers[$i]}:$remote_dir
        scp -i $private_key $sodium_name ${servers[$i]}:$remote_dir
    done
fi


if [ "$1" == "file" ]
then
    # Scripts/miniscripts/kill-ports.sh
    # ip_file: Inference/IP_HOSTS/IP_7
    # Scripts/infer-3pc/inference_3pc.sh
    # ML/inference.cpp
    # remote_dir=Infer-Data/IP_HOSTS
    # Scripts/miniscripts/ping_all_servers.sh
    local_file=Scripts/infer-127pc/inference_127pc.sh
    remote_dir=/home/ubuntu/inferTest/hmmpc-public/${local_file}
    for((i=0;i<=10;i++)); do
        scp -i $private_key $local_file ${servers[$i]}:$remote_dir
    done
fi

# ./Scripts/infer-3pc/inference_3pc.sh   
# ./Scripts/infer-7pc/inference_7pc.sh   
# ./Scripts/infer-11pc/inference_11pc.sh   
# ./Scripts/infer-21pc/inference_21pc.sh    
# ./Scripts/infer-31pc/inference_31pc.sh   
# ./Scripts/infer-63pc/inference_63pc.sh        
# ./Scripts/infer-127pc/inference_127pc.sh     
# ./Scripts/infer-255pc/inference_255pc.sh          

if [ "$1" == "get" ]
then
    # NETWORK {SecureML, Sarda, MiniONN, LeNet, AlexNet, and VGG16}
    NETWORK=SecureML
    BatchSize=1
    NPC=127
    NET=WAN
    PRIME=PR31
    
    remote_file=/home/ubuntu/inferTest/hmmpc-public/Inference/${NETWORK}/test${PRIME}/${NET}/${NETWORK}.b${BatchSize}_${NPC}PC
    # remote_file=/home/liufr/inferTest/hmmpc/Inference/${NETWORK}/test${PRIME}/${NET}/${NETWORK}.b${BatchSize}_${NPC}PC
    # local_file=Inference/${NETWORK}/test${PRIME}/${NET}
    local_file=Inference/${NETWORK}/test${PRIME}_BatInv/${NET}
    # scp ${servers[0]}:$remote_file $local_file
    scp -i ${private_key} ${servers[0]}:$remote_file $local_file
    # scp ${servers[2]}:$remote_file $local_file
fi

if [ "$1" == "chmod" ]
then
    chmod +x ./Scripts/infer-3pc/inference_3pc.sh
    chmod +x ./Scripts/infer-7pc/inference_7pc.sh
    chmod +x ./Scripts/infer-11pc/inference_11pc.sh
    chmod +x ./Scripts/infer-21pc/inference_21pc.sh
    chmod +x ./Scripts/infer-31pc/inference_31pc.sh
    chmod +x ./Scripts/infer-63pc/inference_63pc.sh
    chmod +x ./Scripts/infer-127pc/inference_127pc.sh
    chmod +x ./Scripts/infer-255pc/inference_255pc.sh
fi


if [ "$1" == "test" ]
then
    for((i=0;i<11;i++)); do
        id1=$((6*$i))
        if [ $i -ge 8 ]; then
            delta=$((5*($i - 8)))
            id1=$((48+$delta)) 
        fi
        echo $id1 $id1
    done
    
fi


