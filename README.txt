0 Greetings
:::::::::::

Welcome! Thank you for considering the README file.

CONTENTS

1 What is the sca-toolsuite?
2 Why using the sca-toolsuite?
3 License
4 Documentation
5 Installation & Debugging
6 Examples
7 Thanks to
8 Links
9 Contact

1 What is the sca-toolsuite?
::::::::::::::::::::::::::::

The name `sca-toolsuite' stands for `sca cellular automata toolsuite'.

Earlier, `sca' was used for `sandpile cellular automata'. These are the
cellular automata as introduced by Bak, Tang and Wiesenfeld in 1987. For more
information about cellular automata, please consider [1]. Today, however, this
toolsuite can simulate every cellular automaton which is

  1. synchronous or purely asynchronous and
  2. uses a finite, two-dimensional grid
  
Note that indeterminism is supposed to work.

This toolsuite provides you with a number of tools to

  * create, read or write grids
  * exporting them to TGA, latex (planned) ODS format
  * do math with them like in C++
  * simulate every cellular automata with restrictions listed above
  * very fast routines especially for sandpile automata

Most of the tools can be combined using UNIX pipes (see examples in point 5).

2 Why using the sca-toolsuite?
::::::::::::::::::::::::::::::

Reaons for using it include the following.

  * The sca-toolsuite can simulate cellular automata of any type with both
    high flexibility and high efficiency. Flexibility because you can easily
    specify your cellular automaton like this:

      core/create 20 20 4 |
      ca/ca 'v+(-4*(v>=4))+(a[-1,0]>=4)+(a[0,-1]>=4)+(a[1,0]>=4)+(a[0,1]>=4)'

    Efficiency, because we use a special boost parser to build ASTs, and then
    try to easify those (future feature). Simulators with precompilation are
    usually faster, but the factor should not be more than 3-4.

  * For sandpiles, we provide the (probably) worlds fastest algorithm. Our
    computation time was 68 per cent of the fastest algorithm we knew of
    before. For details, see the file `FAQ'.

  * We have a GUI. It works for all ca, with special functionality for
    sandpiles.

  * The code is quite bug safe. All algorithms were designed both easy and
    fast. Also, we propose the file src/test.sh to test all our programs.

  * We give a lot of documentation: doxygen and --help for all C++ programs.

  * Besides cellular automata, there are many more applications of this
    toolsuite:
      
      * rotor-routing (as described by Priezzhev et al, 1996)
      * image manipulation (future feature), e.g. with filters
      * solving math equations (future feature) or calculating
      * writing C code in scripts
      * spreadsheet
      * running two-dimensional games easily
      * ...
      
3 License
:::::::::

Please see the LICENSE.txt file.

4 Documentation
:::::::::::::::

You can

  * find anything about Documentation in the DOCUMENTATION.txt file.
  * read the FAQ.txt
  * contact me: see `9 Contact'

5 Installation & Debugging
::::::::::::::::::::::::::

Please see the INSTALL.txt file.

6 Examples
::::::::::

There are lots of examples in the file src/test.sh. Here is a small selection:

# use the coords script
core/create 8 8 | math/add `./coords 8 3 5` `./coords 8 5 3`

# manipulate grid via sequences or equations
core/create 8 8 1 | io/field_to_seq | sed 's/ [^ ]* / /g' | io/seq_to_field 8 8
core/create 9 9 0 | math/equation 'x=y||x=8-y'
core/create 20 20 0 | math/equation '(x-8)*(x-8)+(y-8)*(y-8)>16'

# moving a rectangle
core/create 10 10 | math/equation 'x<=4&&y<=4' | io/field_to_seq | math/calc "x+1+10" | io/seq_to_field 10 10

# show 1D avalanche graphically
core/create 9 9 3 | math/add `./coords 9 4 4` | algo/fix l `./coords 9 4 4` | io/avalanches_bin2human 9 | io/seq_to_field 9 9

# using a filter to show which cells decreased
core/create 7 7 3 | algo/throw `./coords 7 3 3` | math/equation "v<=2"

# export to ODS format (OpenOffice)
core/create 4 4 4 | io/to_ods four

# random throw
core/create 1000 1000 | algo/random_throw random 2150000 42 > end_configuration.txt
core/create 8 8 | algo/random_throw random 128 42 l | io/avalanches_bin2human 8 ids

# use the fix command for grids with multiple fields with 4 or more grains
core/create 8 8 8888 | algo/fix s

# burning test
core/create 8 8 1 | algo/is_recurrent
core/create 8 8 2 | algo/burning_test | io/avalanches_bin2human 8  | io/seq_to_field 8 8

# run game of life for 3 steps
core/create 9 6 0 | math/add 10 11 19 20 32 33 34 | ca/ca "`cat ../data/ca/game_of_life.txt`" 3

7 Thanks to
:::::::::::

In alphabetical order, I thank:

  * All people who give support for bost::spirit [6].
  * Oliver Schneider. (TODO: Website?)
  * Philipp Lorenz.
  * Sebastian Frehmel, his diploma thesis at [4].
  * Dr Thomas Worsch [2] @ The Karlsruhe Institute of Technology (KIT) [3].

8 Links
:::::::

[1] http://en.wikipedia.org/wiki/Bak%E2%80%93Tang%E2%80%93Wiesenfeld_sandpile
[2] http://liinwww.ira.uka.de/~thw/
[3] http://www.kit.edu/index.php
[4] http://www.sebastianfrehmel.de/studium/diplomarbeit/
[6] http://boost-spirit.com/home/feedback-and-support/

9 Contact
:::::::::

Feel free to give feedback. My e-mail address is shown if you execute this in
a shell:

  printf "\x6a\x6f\x68\x61\x6e\x6e\x65\x73\x40\x6c\x6f\
  \x72\x65\x6e\x7a\x2d\x70\x72\x69\x76\x61\x74\x2e\x6e\x65\x74\x0a"


