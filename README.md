# rbg2cpp
Regular Boardgames to C++ compiler

This is a part of the Regular Boardgames General Game Playing system. See the official repository: <https://github.com/marekesz/rbg>.

### Usage
Simply clone the parser:
```
git clone --recurse-submodules https://github.com/uicus/rbg2cpp.git
```

Then compile it with
```
make rbg2cpp
```

The compiler processes Regular Boardgame file and outputs single C++17 conforming *.hpp and matching *.cpp. The resulting code depends on boost library in version at least 1.67.0. Note that the compiler itself *does not* depend on the boost library.

Sample usage:
```
bin/rbg2cpp -o reasoner rbgGames/games/breakthrough.rbg
```

### Testing
The package provides an easy way of testing games through flat MC and perft simulations through the provided scripts.
The ```log``` directory should be created first.

Then, e.g.:
```
./run_benchmark.sh breakthrough 10000 0
```
runs the (orthodox) flat MC benchmark for breakthrough over 10 seconds using the default (0) random generator.

Tests can be run directly with a proper ```make``` target.
E.g.:
```
make simulate_breakthrough
```
compiles game description to C++, compiles it together with basic flat MC test and runs it with default parameters. Time package has to be available to run the tests.
