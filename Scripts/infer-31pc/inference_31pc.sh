#!/bin/bash
# make -j8 $EXECUTION 

# paras
# [1]: id
# [2]: test_size = {1, 8}
# [3]: NETWORK
NPC=31
THRESHOLD=15
PlayerID=$1

# PRIME = {PR31, PR61}
PRIME=PR31
# !Params
TEST_DATA_SIZE=1
# NETWORK {SecureML, Sarda, MiniONN, LeNet, AlexNet, and VGG16}
NETWORK=MiniONN
# Dataset {MNIST, CIFAR10, and ImageNet}
DATASET=MNIST
NET=WAN

TRUE_OFFLINE=1
CORES=16

EXECUTION=inference.x

IP_FILE=Inference/IP_HOSTS/IP_$NPC
BASE_FILE=Inference/$NETWORK
OFFLINE_ARG=$BASE_FILE/offline/${PRIME}_offline_b$TEST_DATA_SIZE.txt
Output_file=$BASE_FILE/test${PRIME}/${NET}/${NETWORK}.b${TEST_DATA_SIZE}_${NPC}PC

function clear_file()
{
    if test $PlayerID = "0"; then
        echo $(date) > $Output_file
    fi
}

function kill_ports() {
    id1=$(( 3*$PlayerID ))

    for ((i=0; i<=3; i=i+1)); do
        port=$(( 6000+$id1+$i ))
        id=$(lsof -ti:$port)
        # echo $id
        for j in ${id[@]}; do
            kill -9 $j
        done
    done
}

#exec_once
function exec_once () {
    id1=$(( 3*$PlayerID ))
    id2=$(( 1+$id1 ))
    id3=$(( 2+$id1 ))
    if test $PlayerID = "0"; then
        ./$EXECUTION $id1 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> $Output_file &
        ./$EXECUTION $id2 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id3 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null
    elif test $PlayerID = "10"; then
        ./$EXECUTION $id1 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null
    else 
        ./$EXECUTION $id1 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id2 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id3 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null
    fi
}

clear_file
# kill_ports
if test $PlayerID = "0"; then
    echo onlineTime onlineComm offlineTime offlineComm allComm >> $Output_file
fi
for ((k=1; k<=10; k=k+1)); do
    # kill_ports
    # kill_ports
    exec_once
done
echo "Execution completed"