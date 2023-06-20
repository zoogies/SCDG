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

can i just shutdown engine systems from the game and then reset them? like if i want to change the window size or mode i could end the renderer and then restart it with the new settings? or would that be too slow? i think it would be fine but i need to test it

goal has shifted, we no longer really care about drop in json game data changing the whole entire game easily, it should still be on the mind but its ok to hard code some things into the game code it doesnt all have to be read in the json

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
