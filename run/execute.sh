cd ../src
case $# in
    0) make 'all';;
    1) make $1;;
    *) echo "USAGE: $0 «make-compile»"; exit;;
esac


cd ../run
bash clean.sh
./probSemSharedMemRestaurant