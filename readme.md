# Pandemic Simulator

This is a simple pandemic simulator written in C++. It can simulate the spread of a disease through a population and track the number of infected, recovered, and dead individuals.

## Usage

To use the simulator, compile the code (with the cli.sh script preferably) and run the code. If you want to change hyperparameters such as population, infection rate, etc. you can do so by editing the code, then recompiling.

## Why?

I wrote this script so I could train myself in C++.

## Known bugs/limitations

The code is very unoptimized, and at high populations can take a long time to simulate each day.

## License

You can check the license file for more information.

## Notes

* Currently the code is very unoptimized, and at high populations can take a long time to simulate each day.
* The code is not very well documented, and could be improved.
* The current cli.sh script is not robust, and could be improved by finding the users OMP directory instead of hardcoding it.
* If you want to help, feel free to open an issue or pull request.
* If you are wondering why I don't update the code, its mainly because I keep forgetting to push it.