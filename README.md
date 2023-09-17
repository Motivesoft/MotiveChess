# MotiveChess
A rewrite from perft and motive-chess to be a UCI engine with a turn of speed

# History
## motive-chess
motive-chess was my first sizeable C++ project in a while and was partly just an experiment/side project bit of fun. 

It did the trick in terms of being entertaining to write and somewhat satisfying, but also came with some frustrations:
 * logging that had too much of a performance imact
 * a second logging implementation that was better but still slightly disappointing
 * a move generator that could not be made any quicker. It wasn't bad but it was too slow for any real use
 * a move generator that was fully accurate in its outcome

## perft
perft was a rewrite of my move generator using similar principles to motive-chess but a clean implementation with some distinct changes
* Speed is good - not lightening quick but 4-5 times quicker than motive-chess
* Accuracy is good - passing all the same tests as motive-chess
* Taking the code and shoe-horning it into motive-chess (see branch feature/CHS-100) lost a good amount of its performance as was distinctly disappointing

## rust-perft
My exploration of rust was to port perft - but rust is hard for me to pick up and going is too slow to be rewarding

# New approach
Take perft, copy it and then build the UCI framework around it and hopefully keeping the speed of the original perft.

While we're at it, make it cross-compile onto Linux.