# 2D Drone Sim

A 2D quadcopter simulator in C++ with [raylib](https://www.raylib.com/). You fly a two-engine drone through gravity, drag, and shifting wind, and try to keep it level.

https://github.com/user-attachments/assets/58323266-2f96-4986-b012-368ccad9a88b

## Controls

| Key | Action |
| --- | --- |
| `W` | Raise throttle |
| `S` | Lower throttle |
| `A` | Steer left (more thrust on the left engine) |
| `D` | Steer right (more thrust on the right engine) |

Throttle is a held value between 0 and 1. It stays where you leave it. Hover needs roughly 0.56 throttle, since total thrust at full throttle is 900 against gravity's 500.

## HUD

Total thrust, left and right engine thrust, throttle, speed, wind (signed horizontal speed) and altitude. The camera follows the drone over a 40 px grid, so the grid is your only sense of motion.

## How it works

The drone has two engines, each producing up to half of the maximum thrust. Steering shifts thrust between them, which creates a torque and tilts the drone. Tilt then redirects the total thrust, which is how it moves sideways.

```
thrust_left  = clamp((throttle + steer) * maxThrust / 2, 0, maxThrust / 2)
thrust_right = clamp((throttle - steer) * maxThrust / 2, 0, maxThrust / 2)

alpha  = k * (thrust_left - thrust_right)      angular acceleration
omega += alpha * dt                            then damped: omega *= 1 - c * dt
theta += omega * dt

a.x =  thrust * sin(theta) - drag * (v.x - wind.x)
a.y =  g - thrust * cos(theta) - drag * (v.y - wind.y)
```

Integration is semi-implicit Euler: velocity updates first, then position uses the new velocity.

### Wind

Drag acts on the velocity relative to the air, `v - wind`, so a drone sitting still in a moving air mass is pushed along with it. Wind is horizontal only. A target speed between -100 and 100 px/s is picked at random every 20 seconds, and the current wind eases toward it, so gusts build up and fade instead of jumping. Wind is zero on the ground.

### Ground

Touching the ground zeroes vertical velocity, applies friction to horizontal velocity and resets tilt and spin to zero. There is no crash state yet.

### Constants

| Constant | Value |
| --- | --- |
| Gravity | 500 px/s² |
| Max thrust | 900 |
| Linear drag | 0.5 |
| Tilt torque gain `k` | 0.02 |
| Angular damping `c` | 4.0 |
| Throttle rate | 1.0 per second |
| Steering | ±0.2 |
| Ground friction | 5.0 |

## Build and run

Requires a C++ compiler and raylib 4 or later.

```bash
g++ main.cpp -o drone -lraylib -lm -lpthread -ldl
./drone
```

On macOS and Windows, link raylib's platform libraries as described in the [raylib build guide](https://github.com/raysan5/raylib/wiki).

## Code layout

| Class | Job |
| --- | --- |
| `Drone` | Owns the texture and state: position, velocity, tilt, spin, throttle |
| `Physics` | Reads input, applies thrust, gravity, drag, and wind, handles the ground |
| `Render` | Draws the drone, grid, ground and HUD |

## Credits

- Quadcopter model based on [2D quadcopter simulation](https://mrandri19.github.io/2026/04/03/2d-quadcopter-simulation.html).
- Planar quadrotor dynamics: [Underactuated Robotics, Ch. 3 (Acrobot, Cart-Pole and Quadrotors)](https://underactuated.csail.mit.edu/acrobot.html#section3), MIT.
