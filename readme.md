# Flowers by Allefant

TINS 2020 entry

## Binaries

This package contains Windows and OSX binaries.

Windows: Run the *game.exe* in the *dll* folder.
OSX: Run the *Flowers by Allefant.app*.

## Compile

The game only needs Allegro. I provided my Linux, Windows and OSX
makefiles.

Linux (Ubuntu):
    sudo apt install liballegro5-dev
    make -f Makefile.linux

Windows (Msys2):
    pacman -S mingw-w64-x86_64-allegro
    make -f Makefile.msys2

OSX (homebrew):
    brew install allegro
    make -f Makefile.osx

## Play

You are the bumblebee. All you can do is change its direction, either
by dragging with the mouse or cursor keys.

In the first mission just make sure to stay within 4 yards of the queen
or you have to repeat it. For the reach missions you have to *touch*
the queen, i.e. get really close. For the fetch missions, the "pollen"
are all the floating yellow balls (but not the yellow flower parst). You
can only have one pollen on each leg so 6 total. Hand them over to the
queen.

## Rules

### Theme: Flowers - There should be different flowers in the game, the more the better!
Check!

### make fun of old-fashioned things.
The game is making fun at several occasions of old-fashioned endless
follow quests and ridiculous fetch quests, like we used to hate in 2000s
style MMORPG games.

### Your game must include procedural content. Bonus points if procedural generation is used to create the gameplay environment.
The entire gameplay environment is created procedurally.

### some kind of special text scroller
It's used for all the in-game messages.

## How it was made

I used my collection of code snippets at https://github.com/allefant/land
as well as some .mid playing code from another github repo. All the other
source code was written from scratch during the compo.

For graphics I used my old sphere-object generator, which recently gained the ability to
use bezier deformations. So the flower petals and leaves could be more petal shaped now than
just an ellipsoid and I can have things like bent cylinders for the stems.

The midi file is just "Flight of the Bumblebee" for piano played with
a buzzy sawtooth wave as only instrment. The bumblebee buzz sample I found
somewhere else, I should probably add proper attribution.

## Thanks

Thanks to Amarillion for hosting TINS 2020!

And everyone who participated!

It was a lot of fun, hope to see you all next year :)
