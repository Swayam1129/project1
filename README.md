# CMPUT 350 Project 1: Game Engine + Galaga

**Team:** Swayam Sagar, [teammate name(s)]

Part 1a is tagged `project1a-final`. The current `main` branch is Part 1b.
See `project.txt` for the full description of the implementation, the
Galaga state machine, API changes, team contributions and AI usage.

## How to build and run
    cmake --preset debug
    cmake --build build
    ./build/Project1

## Controls
- **5** insert coin, **1** start
- **A / Left**, **D / Right** move
- **Space** fire (max 2 bullets on screen)

## Notes for TA
- **Window size:** 768x850 instead of 768x1024 so it fits on a laptop screen
  (the spec allows a smaller size). It is one constant (`kHeight`) in `main.cpp`.
- **Template fixes from 1a:** `Rect(float left, float top, float width, float height)`
  passed `(top, left)` to `Point2D`; fixed to `(left, top)`. Added the missing
  `Point2D::Cross(Point2D b) const`.

## AI usage disclosure
Part 1a: no AI used. 
Part 1b: Other AI use (no code):
  We used Claude during Part [1b] only to understand the assignment
  and talk through possible approaches before coding. It did not write
  any code. 
    - “Break this assignment into steps for me.”
    - “Which part should I start first to make it efficient?”
    - “What’s the best way to remove dead objects from a vector?"
    - “Why are x and y swapped in this Rect constructor?”
  I chose the approach and wrote the code myselves.
