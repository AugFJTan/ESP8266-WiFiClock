## Creating custom icons

It is possible to load images from an SD card and print them as icons on the display, but this is not covered in the scope of this project. Instead, we will use a simple trick to treat icons we create as fonts. This not only saves space but is efficient and easy to use.

Take a look at any Adafruit GFX font and you will find that the fonts are listed in an array of hexadecimal values. If you convert each hexadecimal value to binary and put them in a box following a given width and height, you will find that the 1's form the shape of a character corresponding to its ASCII value. With this idea in mind, we can create images in place of these characters.

As of right now, I have written a very rudimentary program in C++ called [ArraytoHex](../ArraytoHex/ArraytoHex.cpp) that takes the C output from [Piskel](http://www.piskelapp.com/) and uses it to create a hex array. Before using it, you first have to create a drawing on Piskel. 

![Piskel](/images/Piskel.png)

The current `ArraytoHex` program currently only supports black and transparent, but you can edit it according to your needs.

![Piskel](/images/Export.png)

Next, click on the `EXPORT` tab, select `Others` and click `Download C file`. From the file that you just downloaded, copy the array into `pic.txt`. You will find that the default `pic.txt` that comes with the repository has the array for the LOL face in the images above.

If you are on Linux, you can compile `ArraytoHex.cpp` on the command line by typing

```shell
g++ ArraytoHex.cpp -o ArraytoHex
```

Run the executable with `./ArraytoHex`. The output should look something like this:

![ArraytoHex](/images/ArraytoHex.png)

There should be an output file called `hex.txt` with the hexadecimal output as shown on the command line. Copy its contents and create a header file that is similar to a font header file. Use this template to help you get started:

```c
const uint8_t MyIconsBitmaps[] PROGMEM = {
  //hex output goes here
  0x00, 0x0f, 0xf0, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x01, 0xff, 0xff, 0x80, 
  0x03, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xf0, 
  0x1e, 0x1f, 0xf0, 0xf8, 0x39, 0x07, 0xc8, 0x3c, 0x37, 0x03, 0xb8, 0x1c, 
  0x77, 0x03, 0xb8, 0x1e, 0x6f, 0x81, 0x7c, 0x0e, 0x6f, 0xc1, 0x7e, 0x0e, 
  0xef, 0xfd, 0x7f, 0xef, 0xef, 0xfd, 0x7f, 0xef, 0xf0, 0x03, 0x80, 0x1f, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x3f, 
  0xe0, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x1f, 0x60, 0x00, 0x00, 0x1e, 
  0x70, 0x00, 0x00, 0x3e, 0x70, 0x00, 0x00, 0x3e, 0x38, 0x00, 0x3c, 0x7c, 
  0x3c, 0x00, 0xff, 0x7c, 0x1e, 0x01, 0xfe, 0xf8, 0x0f, 0x01, 0xfd, 0xf0, 
  0x07, 0xc3, 0xf3, 0xe0, 0x03, 0xf0, 0x0f, 0xc0, 0x01, 0xff, 0xff, 0x80, 
  0x00, 0x7f, 0xfe, 0x00, 0x00, 0x0f, 0xf0, 0x00, 
};

const GFXglyph MyIconsGlyphs[] PROGMEM = {
  {   0, /*width=*/32, /*height=*/32, /*xAdvance=*/32, 0, 0}, //'0' LOL face
};

const GFXfont MyIcons PROGMEM = {
  (uint8_t  *)MyIconsBitmaps,
  (GFXglyph *)MyIconsGlyphs,
  /*First ASCII value*/0x30, /*Last ASCII value*/0x30, /*yAdvance*/32 };
```

Save the file as `MyIcons.h` or whatever you fancy and place the file in the `Fonts` folder under the Adafruit GFX library folder. To use the icon and output it on the display, in your sketch simply set the font and print it.

![Small LOL](/images/SmallLOL.jpg)

And viola! You can now display custom icons on your display screen. It's a little small, but we can enlarge it with the `setTextSize` function:

![Big LOL](/images/BigLOL.jpg)
