
for ((i=0; i<=255; i=i+1)); do
    port=$(( 5000+$i ))
    id=$(lsof -ti:$port)
    echo $id
    for j in ${id[@]}; do
        kill -9 $j
    done
done