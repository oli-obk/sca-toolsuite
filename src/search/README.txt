This subdirectory deals with searching for all possible resulting
configurations from simulating CA.

You can work with it like this:

# Go into this directory
cd search

# Generate the CA's table file
cat ../../../data/ca_by_grid/circuit.txt | ../ca/converter grids table > circuit.tbl

# Run the search algorithm
./search circuit.tbl 3 nodump greedy pipe < ../../../data/search/crossing_small.txt > results.dat

# Evaluate the results
./eval help < results.dat

# A complete example to produce latex output:
cat ../../../data/search/crossing_small.txt |
./search circuit.tbl 3 nodump greedy pipe |
./eval tex |
../io/tik complete

