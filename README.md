# Wilted Chess Engine

TheTilted096

Base of future chess engine projects

# To-Do's and Notes

profit
mald over search

remember to change the version no in FOUR places

investigate infrequent time losses

rethink standard library include locations

isLegal()

inspect and slightly clean up smp code?

movelist/scoredmovelist wrapper

pickmove instead of sort all at once

# Versioning Scheme

4th - Non functional

3rd - Small patch

2nd - Moderate update

1st - very important

dev names will be x-x-x-x-dev, updates numbers if passed

major plans: 

1.0.0.0 - the end of the beginning

2.0.0.0 - NNUE

# specifications

uses PEXT for slider generation, no other supported yet

supports only shredder fen, plays 960 if UCI_Chess960 is set. otherwise, undefined behavior

'go depth' will only limit the main thread

'go nodes' will restrict to single thread


