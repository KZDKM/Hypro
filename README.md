# Hypro

> If you are big enough of a lunatic to shell out the money for a post-2021 Macbook Pro *AND* daily drive Asahi Linux with Hyprland on it: fucking why + touch some damn grass + find something better to do with your life. But you might find this plugin useful get the notch out of your way while you are in fullscreen

A Hyrpland plugin that adds paddings / reserved areas to fullscreen windows. Applicable to any device / setup that involve irregular screens, or those which had half of their screen smashed while its user thinks its a good idea to carry on using it.

## Installation

### Manual

To build, have hyprland headers installed and under the repo directory do:
```
make all
```
Then use `hyprctl plugin load` followed by the absolute path to the `.so` file to load, you could add this to your `exec-once` to load the plugin on startup

### Hyprpm
```
hyprpm add https://github.com/KZDKM/Hypro
hyprpm enable Hypro
```

## Configuration

- `plugin:hypro:monitor` monitor to apply the padding to (e.g. `eDP-1`)
- `plugin:hypro:top` top padding in pixels
- `plugin:hypro:bottom` bottom padding in pixels
- `plugin:hypro:left` left padding in pixels
- `plugin:hypro:right` right padding in pixels

- You might also find the `addreserved` feature from hyprland and setting your panel to non-exclusive useful.