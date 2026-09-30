# CMPUT 350 Project 1a: Game Engine + Galaga

**Team:** Swayam, 

## How to build and run
    cmake --preset debug
    cmake --build build
    ./build/Project1

In `galaga/main.cpp`, set `mBallSsample = true` to run the bouncing-ball
engine demo, or `false` to run Galaga.

**Controls:** A / D to move, Space to fire (max 2 bullets on screen).

## Notes for TA
- **Window size:** Galaga runs at 768x850 instead of 768x1024 because 1024 px
  does not fit my laptop screen (the spec allows a smaller size). The
  height is a single constant (`kHeight`) in `main.cpp`.
- **Template bug fixed:** `Rect(float left, float top, float width, float height)`
  passed `(top, left)` to `Point2D`, swapping x and y. Fixed to `(left, top)`.
- **Added missing spec function:** `Point2D::Cross(Point2D b) const` was in the
  spec but not in the template.
- `Line` functions and `Rect::operator|=(const Line&)` are left for Part 1b.
- Removal of dead objects uses swap-and-pop (O(1) per removal, no gaps);
  object order is not preserved, as allowed by the spec.

## AI usage disclosure

I did not use AI