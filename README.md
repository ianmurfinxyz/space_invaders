<p align="center">
  <img src="img/invaders.png" alt="crab octopus squid cuttle"/>
</p>

# Space Invaders Part II Clone :space_invader:

This is clone of the 1979 game space invaders part II implemented with a single file custom 
engine using SDL2 and opengl 2.1 (because my computer is kinda old :persevere:). SDL2 is used 
for window management and to create the opengl context whereas opengl is used for all rendering.

<p align="center">
  <img src="img/splash.gif" alt="splash screen and menu"/>
</p>

## Rendering

The engine uses a somewhat novel approach to rendering (because I thought it would be fun, even
if not fast on modern hardware) in that all fonts and sprites are rendered as pure bitmaps using 
glBitmap and color is added by simply setting the draw color is the context. As a consequence 
all sprite assets and font glyphs are just arrays of 1s and 0s saved to files. 

To handle multiple screen resolutions all the entire game world is scaled during initialisation
and all bitmaps are scaled to fit the world.

## Compilation and Dependencies

I have only tested this game on linux so it may not work on windows. The only libraries required 
to compile however are SDL2 and an opengl implementation. On arch linux these can simply be
installed with,

```shell
$ pacman -S sdl2
$ pacman -S mesa
```

then you can compile by running make,

```shell
$ make
```

## TODO

This work is not yet fully complete, I still need to:

* Add a sound module to pixiretro and add music and sound effects.
* Implement a couple missing gameplay systems such as the mystry ship which drops reinforcements.
* Add an end screen where you can insert your name upon achieving a high score.
* Fix that dodgy 'b' on the splash screen! :smirk:
