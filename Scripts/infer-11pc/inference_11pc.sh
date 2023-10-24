#!/bin/bash
# make -j8 $EXECUTION 

# paras
# [1]: id
# [2]: test_size = {1, 8}
# [3]: NETWORK``
NPC=11
THRESHOLD=5
PlayerID=$1
TEST_DATA_SIZE=1

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
    for ((i=0; i<=$NPC; i=i+1)); do
    port=$(( 5000+$i ))
    id=$(lsof -ti:$port)
    # echo $id
    for j in ${id[@]}; do
        kill -9 $j
    done
done
}

#exec_once
function exec_once () {
    # kill_ports
    if test $PlayerID = "0"; then
        ./$EXECUTION $PlayerID $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> $Output_file
    else
        ./$EXECUTION $PlayerID $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null
    fi

    # kill_ports
    # id1=$(( 6*$PlayerID ))
    # id2=$(( 1+$id1 ))
    # id3=$(( 2+$id1 ))
    # id4=$(( 3+$id1 ))
    # id5=$(( 4+$id1 ))
    # id6=$(( 5+$id1 ))
    
    # if test $PlayerID = "0"; then
    #     echo $PlayerID
    #     echo $id1 $id2 $id3 $id4 $id5 $id6
    #     ./$EXECUTION 0 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG >> $Output_file &
    #     ./$EXECUTION 1 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG >> /dev/null &
    #     ./$EXECUTION 2 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG >> /dev/null &
    #     ./$EXECUTION 3 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG >> /dev/null &
    #     ./$EXECUTION 4 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG >> /dev/null &
    #     ./$EXECUTION 5 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG >> /dev/null 
    # else 
    #     echo $PlayerID
    #     echo $id1 $id2 $id3 $id4 $id5 
    #     ./$EXECUTION 6 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG  &
    #     ./$EXECUTION 7 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG >> /dev/null &
    #     ./$EXECUTION 8 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG >> /dev/null &
    #     ./$EXECUTION 9 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG >> /dev/null &
    #     ./$EXECUTION 10 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG >> /dev/null 
    # fi
}

clear_file
# kill_ports
if test $PlayerID = "0"; then
    echo onlineTime onlineComm offlineTime offlineComm allComm >> $Output_file
fi
for ((k=1; k<=10; k=k+1)); do
    exec_once
done
echo "Execution completed"