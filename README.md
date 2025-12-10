![Logo](./assets/logo.jpg)

**Extase** is a modern, lightweight terminal-based text editor engineered from the ground up in pure C. It is built with a zero-dependency philosophy, avoiding libraries like ncurses in favor of the raw POSIX terminal API. 

Extase is currently compatible with all modern POSIX systems, with native Windows support on the roadmap.

## Installation
Because Extase relies purely on the POSIX standard with no external dependencies, building it is fast and straightforward. You only need a C compiler (GCC or Clang) and Make tool.

#### Build from source
First, download the repository from github
```
git clone https://github.com/alechiozza/extase.git
```

Then, compile it with
```
cd extase
make
```
**(Optional)** You can install extase globally on your system with
```
sudo make install
```
Or uninstall it with
```
sudo make uninstall
```

To run the editor, you just need to type
```
extase <filename>
```

## Screenshots

![Screenshot](./assets/screenshot1.png)

## Keybindings

### Normal Mode

These are the keybinds currently supported in NORMAL mode.

| Keybind | Action |
| ------- | ------ |
| Ctrl+Q  | Close the current window or quit if only one window remains |
| Ctrl+S  | Save |
| Ctrl+F  | Find a keyword |
| Ctrl+L  | Toggle the line numbers at the left of the text buffer |
| Ctrl+T  | Toggle tab mode. If the tab mode is set on space mode, every TAB character is highlighted |
| Ctrl+O  | Open the file picker |
| i       | Switch to insert mode |
| r       | Toggle relitive line numeration |
| o       | Switch to insert mode and add a new line |
| 0       | Move the cursor at the beginning of the line |
| $       | Move the cursor at the end of the line |
| dd      | Delete the entire line |
| d0      | Delete everything at the left of the cursor |
| d$      | Delete evrything at the right of the curosr |
| :       | Open the editor command shell |
| Ctrl+W  | Switch to window control mode |

**Way more keybinds will be added in the future.**

### Window control mode

| Keybind | Action |
| ------- | ------ |
| + or =  | Increase the size of the active window |
| -       | Decrease the size of the active window |
| > and < | Resize the window (in the direction of the arrow pressed) |
| j, k, l, q and arrows | Move to the up/down/left/right window |
| q       | Close the window or quit the editor if only one remains |
| v       | Split the windows vertically |
| s       | Split the windows horizontally |

## Contributing

Contributions are welcome and appreciated! Please use the [Issue Tracker](https://github.com/alechiozza/extase/issues) to report bugs, suggest new features, or ask questions about the codebase.

We are open to Pull Requests from the community. If you are interested in fixing a bug or implementing a feature, feel free to submit a PR :)
