## Switching between different fonts

As a rule of thumb, when switching between different fonts on the display, one should follow the order of:

```
Set Font -> Set Cursor -> Print Text
```

This is because when switching fonts the subsequent font follows the same baseline as the previous font. While this may be desirable in some cases, when testing the clock I encountered a slight issue due to the text appearing to shift slightly higher when it updated.

See the very bottom of [fontconvert.c](https://github.com/adafruit/Adafruit-GFX-Library/blob/master/fontconvert/fontconvert.c) for more details.



