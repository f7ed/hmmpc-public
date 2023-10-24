#!/bin/bash
# make -j8 $EXECUTION 
# paras
# [1]: id
# [2]: test_size = {1, 8}
# [3]: NETWORK
NPC=63
THRESHOLD=31
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
    id1=$(( 6*$PlayerID ))
    if [ $PlayerID -ge 8 ]; then
        delta=$((5*($PlayerID - 8)))
        id1=$((48+$delta))
    fi

    for ((i=0; i<=6; i=i+1)); do
        port=$(( 5000+$id1+$i ))
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
    id1=$(( 6*$PlayerID ))
    if [ $PlayerID -ge 8 ]; then
        delta=$((5*($PlayerID - 8)))
        id1=$((48+$delta))
    fi
    id2=$(( 1+$id1 ))
    id3=$(( 2+$id1 ))
    id4=$(( 3+$id1 ))
    id5=$(( 4+$id1 ))
    id6=$(( 5+$id1 ))
    
    if test $PlayerID = "0"; then
        # echo $PlayerID
        # echo $id1 $id2 $id3 $id4 $id5 $id6
        ./$EXECUTION $id1 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> $Output_file &
        ./$EXECUTION $id2 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id3 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id4 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id5 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id6 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null 
    elif [ $PlayerID -ge 8 ]; then
        # echo $PlayerID
        # echo $id1 $id2 $id3 $id4 $id5 
        ./$EXECUTION $id1 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id2 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id3 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id4 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id5 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null
    else 
        # echo $PlayerID
        # echo $id1 $id2 $id3 $id4 $id5 $id6
        ./$EXECUTION $id1 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id2 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id3 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id4 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id5 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null &
        ./$EXECUTION $id6 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES >> /dev/null
    fi
}

clear_file
# kill_ports
if test $PlayerID = "0"; then
    echo onlineTime onlineComm offlineTime offlineComm allComm >> $Output_file
fi
for ((k=1; k<=10; k=k+1)); do
    # kill_ports
    exec_once
done
echo "Execution completed"
