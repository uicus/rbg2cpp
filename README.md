# rbg2cpp
Regular Boardgames to C++ compiler

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
The package provides an easy way of testing games through flat MC and perft simulations. They can be run with proper ```make``` target.

E.g.:
```
make simulate_breakthrough
```
compiles game description to C++, compiles it together with basic flat MC test and runs it with default parameters. Time package has to be available to run the tests.
