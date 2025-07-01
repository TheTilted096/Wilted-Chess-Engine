# Wilted Chess Engine

TheTilted096

Base of future chess engine projects

# To-Do's and Notes

sanity check search, add PV Table

write a perftsuite function

move some attack functions from Generator to Position

rethink standard library include locations

isLegal()

lazy updates...?

perhaps neaten up deduceCastling() to remove some conditionals

perhaps regain 5-10% perft speed (slightly more branching, wrapper types)

# Versioning Scheme

4th - Non functional

3rd - Small patch

2nd - Moderate update

1st - very important

# specifications

uses PEXT for slider generation, no other supported yet

supports only shredder fen, plays 960 if UCI_Chess960 is set. otherwise, undefined behavior


