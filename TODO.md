# TODO

## FEATURES

### ENGINE

- [ ] dev console
- [ ] hot reload (possible, python C pid monitoring script)
- [ ] test logging on windows
- [ ] logging before SDL init
- [ ] outputs when opening any type of file or image
- [ ] allow text outlines
- [ ] force errors and warnings to be all caps
- [ ] go through and change naming conventions to be underscore lowercase instead of camel case and match c style better

### GAME

- [ ] exe icon
- [ ] rpc generic functions & heart in corner
- [ ] custom cursor
- [ ] enums for common sizes of things (button sizes, text sizes, etc)

## BUGS

### WINDOWS

- [ ] windows sometimes cant launch from explorer bug

### GENERIC

### ASSUMED

- [ ] im guessing that linux build without deps does not work (windows needs dlls)

## MISC

- [ ] look into optimizations
- [ ] look into bundling everything into one exe
- [ ] every other todo in project comments
- [ ] macOS port? (difficult bc need dev tools but will look into for ben)
- [ ] convert all spaces to tabs and setup auto formatter

- json loads in static and starter data, then game looks towards scene flow descriptor in json which is a list of events and awaits which control how the game flows, for non standard things (unprecedented events like minigame chess) control is explicitely handed over to game function to handle and then resolved back to the scene flow descriptor

some sort of level editor where i can place buttons and text and stuff and then it will generate the json for me

6/17/2023:

- hunt down and optimize what is making the game hang when adding a lot of new textures and buttons, cache textures and stuff to combine and speed up
- figure out key value pair state, how does the game flow work, callback functions accessing globals? there will be dynamically loaded variables that need referecned?

state and other thoughts:

- caching common created textures to eliminate a lot of overhead
- less debug output lines for creating and destroying renderobjects unless the op fails
- better text fitting, it should not stretch it should fill its bounds normal ratio and shrink if needed
- playtime counter
- text effects and outlines
- animations and videos

- i would really like to be sending parameters to genericize things with callbacks

- level editor from flag
- error trapping void functions (into int return status)

- code quality pass
- switch from type parameter generic get type from json function
- font lookup table in gamedata specifiers in renderObjects
- optimization of game data reading, persist game data in memory so less read write
- logging persist ?

- function pointers callback
- port settings to gamedata.json
- text keep aspect ratio forced (pass bool param ignore)
- button bg texture lookup key
- missing common bool fields assumed (prototype?)

is there a way to have generic key values in json that get looked up in common table so dont have to repeat common vals (button alignement etc)

- list aspect ratio in settings
- settings confirm switch timer
- enable resizable window mode
- abstract to groupings for json:

"group":{
    "label":"group",
    "items":{...}
}

- identifier or description in json
- check new setting is not existing setting before running

check each object for a prototype and apply defaults from prot, if not existing handle as if no prot

- game data key lookip for common positions and sizes
- construct game object in data.c

- optional cli arg for log level

- load all fonts and textures cached into lookup table or lazily load them when needed (prob lazy load)
  - happenning in the engine caching the textures and fonts to a table
    - take in a arg for caching or try to intelligently decide? (could have int counting how many uses and if its one and we destroy the only we remove it from the table)

game reads fonts and colors into itself and then when gamedata references them, it passes the already loaded font or color to the renderobject

- cache flag on certain json obj fields?

FIX:

- engine relies on one font present
  - solution: pass in font

- hashmap with void pointers instead of kvp for all game tracking stuff (or possibly defined in engine and extended to game) can be used to cache in engine and game can use to lazily load fonts and colors
  - how to free fonts and colors when no longer needed? is it valid to drop all loaded fonts and colors when scene changes? (probably)

- debug log entries should have more verbose information on calling function with it args

TODO RN:

1. startup font is used only in update text. find a way to check cache and load it from only identifier
   1. when would it ever not exist in cache but text being updated? change scene but object persist? update text should just take in the font and color just in case.
   2. make update text a more generic function taking in font and color and value and id (solves this issue)
   3. that shit should go in graphics
   4. updating text forces item to be tracked, so it means the color and text it relies on has to be cached because automatic cache clear happens on load which also destroys tracked things (right now) this is true unless someone manually cache clears mid scene.
2. caching (engine) (lazily)
   1. ENGINE
      1. hashmap
3. baking together static groups of renderObjects intelligently
4. callback system
   1. reading from gamedata
   2. constructing struct passed with wrapper fn pointer
   3. recieve in wrapper in game and switch statement with enum type and struct params, dispatch to correct handler
5. clean up and split a lot of rendering and organization code from the game (which should only have game loop and entry stuff)
6. fix all memory leaks from valgrind, lot of jannson stuff
7. clean up code comments and organization

thoughts:
hashmap game side or use engine exposed caching tracking hashmap

minute optimizations:

- fonts specify size in json

construct scene needs to be better structured, it should take in all keys, should manage its own music too since we are passing keys in anyways

cleanup all color usage (it needs it baaaad)

can the game font caching and color caching code be moved to engine? game registers fonts and colors there and just passes id? but game should track its own shit maybe?

callbacks first because can benchmark caching improvements

steamworks SDK

only supports limited parameters for callback (type and amount)

json_t inside callbacks?? why assign manually just to read back out when you can have it anyways?

jannsonn forced for callbacks?

engine graphics should NOT have callback code in it

clean up ugly conflicting enum and var names

we know this commit stems from the LL caching datastructure (its not callback)

- probably a memory issue

CACHING:

- need some game side functions for tryGetFont tryGetColor which will look at cache first, then create and add to cache if not existant

removeRenderObject appears to be skipping?

- maybe not, if sorted by depth the buttons can be inserted in weird ways (reverse)

- everything in jansson is wrong

go back to stable fix jansson then merge

go through every single time jannsonn is used and fuck with refs

notes 6/24:

- fix jansson memory valgrind warnings, they are probably the root of the issue. the segfault linked list corruption likely comes from the internals of jansson

fix pointer notation everything should have p

GOOD IDEA ACTUALLY HOLY:

TODO: do this when kvp/hashmap/global game state management implemented

maybe the best decision is to hold our json data globally so we only open and ref it once and dont worry about decref

work on segmenting engine into its own code, submodule? build it separate and link with game (eventully)

impl engine detatched from jansson callbacks.

- unions in a struct (like 10) for parameters
  - allow as fallback null pointers, so anyone can send in unsupported types. (is this possible?)

main menu is double loading

6/29/23 NOTES:

- callback handler needs work. we need to pass struct of union params and not json_t (maybe can be pushed off to later task but why not just do now)
- actual implementation of callback system with switches
- volume controls and updating text
- take measurements of performance and see if we can improve after caching
- KVP hashmap global state system for caching
- engine baking layers together optimization

WHAT IVE BEEN WORKING ON:

- settings buttons changing things
- ive discovered that we actually should shut down the graphics subsystems and then re initialize them to make these changes without destroying everything
  - implement the ability to destroy window + renderer and create them again

- save on exit
- update screen res when fullscreened (exists in game)

SETTINGS TODO:

- allow custom resolutions

change all exit(1) non criticals to returns so we can keep going.

debug_verbose keyword: go through and change whats logging and whats not (so many debugs rn)

soft reload scenes without restarting music (useful for settings or seamless reload)

7/1/23:

- we added a bunch of backend stuff to change window mode fps and res, but its all pretty garbage so will have to look at later.
- i think itll be easier to implement this with some sort of global game state manager (implement this first)
- do the caching stuff before we worry about state
- then do the engine caching (eventually) after state

make our own hashmap implementation

pre optimization settings load takes 250ms
font caching cut 40ms

cache size debug overlay counter
