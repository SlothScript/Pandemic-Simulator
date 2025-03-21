# CHANGE SCRIPT TO FIND OMP DIRECTORY INSTEAD OF HARDCODING MINE
g++ -std=c++11 -I/usr/local/opt/libomp/include -L/usr/local/opt/libomp/lib -Xpreprocessor -fopenmp -lomp -o pandemic-sim pandemicCLI.cpp
if [ $? -eq 0 ]; then
    echo "Compiled CLI"
    ./pandemic-sim
    rm pandemic-sim
else
    echo "Compilation failed"
    exit 1
fi