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

- [ ] main menu event handling and changing settings
- [ ] loading from game data
- [ ] generic functions for reading values from save data
- [ ] exe icon
- [ ] rpc generic functions & heart in corner
- [ ] custom cursor
- [ ] enums for common sizes of things (button sizes, text sizes, etc)

## BUGS

### WINDOWS

- [ ] command line color codes dont display
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