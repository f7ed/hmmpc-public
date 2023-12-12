#!/bin/bash
# Make an inference in the setting of 3PC
# ./Scripts/inference.sh 3

MAX_PLAYERS=$1
TEST_DATA_SIZE=1
# PRIME = {PR31, PR61}
PRIME=PR31
# NETWORK = {SecureML, Sarda, MiniONN}
NETWORK=SecureML
# Dataset = {MNIST}
DATASET=MNIST
# True Offline: 
TRUE_OFFLINE=1
CORES=4

IP_FILE=Inference/IP_HOSTS/IP_LOCAL
BASE_FILE=Inference/$NETWORK
Output_file=$BASE_FILE/$NETWORK.P0
OFFLINE_ARG=$BASE_FILE/offline/${PRIME}_offline_b$TEST_DATA_SIZE.txt


OnlyOutputP0=1

EXECUTION=inference.x

make -j8 $EXECUTION 

function gen_ip () {
    echo > $IP_FILE
    for ((i=0; i<$1; i=i+1)); do
        echo "127.0.0.1" >> $IP_FILE
    done
}

#clear_file n
function clear_file()
{
    echo $(date) > $Output_file
}

function kill_ports() {
    for ((i=0; i<=$1; i=i+1)); do
        port=$(( 6000+$i ))
        id=$(lsof -ti:$port)
        # echo $id
        for j in ${id[@]}; do
            kill -9 $j
        done
    done
}


#exec_once n t
function exec_once () {
    gen_ip $1

    kill_ports $1

    for ((i=1; i<$n; i=i+1)); do 
        if test $TRUE_OFFLINE = "0"; then
            ./$EXECUTION $i $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES>> /dev/null &
        else
            ./$EXECUTION $i $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES>> /dev/null &
        fi
    done

    # For P0
    if test $TRUE_OFFLINE = "0"; then
        ./$EXECUTION 0 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES 
    else
        ./$EXECUTION 0 $IP_FILE $THRESHOLD $NETWORK $DATASET $TEST_DATA_SIZE $TRUE_OFFLINE $OFFLINE_ARG $CORES
    fi
}

function one_iteration () {
    n=$MAX_PLAYERS
    THRESHOLD=$((n/2))
    exec_once $n $THRESHOLD
}


clear_file $MAX_PLAYERS
for ((k=1; k<=1; k=k+1)); do
    if test $TRUE_OFFLINE = "1"; then
        echo "[Note: onlineTime and offlineTime measure the online and offline time of P0 by default, respectively.]"
        echo "[Note: onlineComm, offlineComm, and allComm measure the average of communication bytes sent by all parties.]"
        echo "onlineTime onlineComm offlineTime offlineComm allComm"
    fi
    one_iteration
done

echo "Execution completed"