# Super Mario OpenGL
Mario and Boo have to collect stars from the island, ship, and the hidden room. Mario can jump, change color and go down a pipe to the hidden room, and Boo can fly.

## Key Features
- **Blending**: Implemented alpha blending for transparent objects.
- **Face Culling**: Optimized rendering performance by culling back-faces of 3D models.
- **Advanced Lighting**: Utilized the Blinn-Phong lighting model.
- **Cubemaps (Skybox)**: Implemented environment mapping using cubemaps for realistic backgrounds.
- **Instancing**: Employed instancing techniques for efficient rendering of multiple instances of the same model (coins).
- **Framebuffers**: Utilized framebuffers to apply sharpen effect.
- **HDR and Bloom**: Implemented High Dynamic Range (HDR) rendering and bloom effect.
- **Normal Mapping**: Applied normal mapping techniques for increased surface detail without additional geometry.

## Technologies Used
- C++
- OpenGL

## Instructions
1. `W` `S` `A` `D` -> Camera movement
2. `↑` `↓` `←` `→` -> Mario/Boo movement
3. `SPACE` -> Mario jumps / Boo flies up
4. `LEFT SHIFT` -> Mushroom goes back to its box / Boo flies down
5. `C` -> Change the character
6. `1` -> Bloom on/off
7. `2` -> HDR on/off
8. `3` -> Sharpen effect on/off
9. `Q` `E` -> Exposure +/-
10. `F` -> Flashlight on/off

## Demo Video
[Link](https://youtu.be/UnUEZbtmJPE)

