# translate2ME
Offline dictionary for easy integration/accessibility

For the impatient: if you just run the binary and nothing happens or you'll get a notification that there is already an instance of the program running, please take a look on Installation & Usage. 
You can toggle the main window by pressing WIN+CTRL+J. If the main window pops up on your screen, you can press and hold ALT key. This displays an overview of options and buttons you can use in this program. E.g. ALT+Q will exit the program.

# Introduction

When I wrote the first version of this program I was experimenting with minimalism. Not in a sense of KISS (keep it simple stupid) or reducing the program to a minimum size, but more in 1920s Bauhaus attitude: that the program fulfills the function and purpose in the most optimized way and does not carry any overhead nor is bound to the restrictions of the OS's user interface. Functionality comes first and is the driver for everything else.

The intention was to build something, that improves productivity. More concrete, a dictionary and the function is to provide the information when needed, without distracting too much from the main activity (like writing an email/reading a technical paper or whatever).
I found it very annoying to open a browser, click the search bar of some online dictionary - and of course wait and keep the patience until all this loads up. Instead the application should just be available to enter data or present information that I was looking for as quick as possible and that meant
- offline dictionary (=prevent the loading times of browser or online resources) - but with the option of accessing online information in case the available data is not sufficient
- full keyboard control / hotkey availability (=no mousemovement/timewaster & access without startmenu or UI interaction) - I only need this information when reading or writing and in such case my hands are on the keyboard; if they are not, then I need to touch the keyboard anyways to enter the search phrase
- background application (=the application is always running in the background and ready to serve/no timewaster of loading the dictionary or serving user interface tasks). Keep waiting times as short as possible.
- when using the application it is always on top-most visibility and takes the full screen (=there's no need of a windows-like sorting of windows and there's no such need to have it behind another windows: either I use it, then it is visible; or I don't use it and then I don't need to see it nor does it need to fill my taskbar). No distractions.

Back in this time I was building a set of two other programs (snoopy & spreadx), mainly to mitigate the shortcomings of the Windows operating system. Unfortunately, this operating system did not improve over the years and still I'm obliged to use it in context of my employment. 

# Installation

You need to download and extract the release binary and you'll need a dictionary. The default configuration file is set for (only) using beolingus https://dict.tu-chemnitz.de/index.html.en which can be downloaded and shared under GPL.
Dict.cc is a great source and also provides offline-dictionary data, which you can download for personal use. You can download these by following "8. Vocabulary Database" link here (https://www.dict.cc/?s=about%3Awordlist)

If you find this program useful or you often use it, then I'd recommend to add it to your windows autostart programs. On Win11 you can do this by WIN+R -> Enter "shell:startup" -> Create a link to translate2Me.

# Usage

If you are using the default configuration bundled to the binary and you've stored the dictionary under the "dat"-subfolder, then only the beolingus dictionary is available.
There is a second configuration file in the folder that gives an example of how other dictionaries can be used and integrated.

When the program is started you can show the main window, by pressing CTRL+WIN+J. When you press ESC or the window focus is set to another window, the main window will disappear.

When the main window is visible you can
- Just enter a search string and press enter
- Press ALT+Backspace to clear the search string entry
- Press ALT+ENTER to open a browser window and search for the entered search string online
- If you have more search results than fit on the page you can scroll up and down with PageUp + PageDown keys

If the main window is visible and you hold the ALT key, you will see an overview of available hotkey inputs and the following options
- ALT+Q will quit the program
- ALT+J will enable/disable language specific substitutions (can speed up search)
- ALT+K will toggle case matching for search results
- ALT+L will change search type (toggle in between RegEx-, Literal- and Wildcard-Search)
- ALT+1..ALT+9 will switch to another dictionary if you have enabled in the configuration file

# Configuration File

By default the program will read configuration from the file config.cfg which is in the same folder as the executable. You can edit this file with a regular texteditor and major configuration, like colors fontsizes and the hotkey to toggle the window can be configured here.
To add a dictionary to the program you can use the call LANG_CFG() in this file. It takes two arguments - a descriptor/name which should be displayed and another script file, which contains the language specific data.
For examples on howto use this, take a look on the commented config file examples.
