# Wilted Chess Engine

TheTilted096

Base of future chess engine projects

# To-Do's and Notes

remember to change the version no in 3 places

remember to do the mate score in tt update

possibly create compound class between Generator and Position

movelist/scoredmovelist wrapper

pickmove instead of sort all at once

switch to safer method of exiting search

re organize, maybe a sub-folder

investiate slowdowns, -(5-10%) perft, slower incremental psqt

write a perftsuite function

rethink standard library include locations

isLegal()

lazy updates...?

perhaps neaten up deduceCastling() to remove some conditionals

# Versioning Scheme

4th - Non functional

3rd - Small patch

2nd - Moderate update

1st - very important

dev names will be x-x-x-x-dev, updates numbers if passed

# specifications

uses PEXT for slider generation, no other supported yet

supports only shredder fen, plays 960 if UCI_Chess960 is set. otherwise, undefined behavior

'go depth' will only limit the main thread

'go nodes' will restrict to single thread


