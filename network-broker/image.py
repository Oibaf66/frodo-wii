#!/usr/bin/env python

import Image, sys, functools

DISPLAY_X = 0x180
DISPLAY_Y = 0x110

SCREENSHOT_SCALE = 4
SCREENSHOT_X = DISPLAY_X / SCREENSHOT_SCALE
SCREENSHOT_Y = DISPLAY_Y / SCREENSHOT_SCALE

SCREENSHOT_SIZE = (SCREENSHOT_X * SCREENSHOT_Y) / 2

reds = [0x00, 0xff, 0x86, 0x4c, 0x88, 0x35, 0x20, 0xcf, 0x88, 0x40, 0xcb, 0x34, 0x68, 0x8b, 0x68, 0xa1]
greens = [0x00, 0xff, 0x19, 0xc1, 0x17, 0xac, 0x07, 0xf2, 0x3e, 0x2a, 0x55, 0x34, 0x68, 0xff, 0x4a, 0xa1]
blues = [0x00, 0xff, 0x01, 0xe3, 0xbd, 0x0a, 0xc0, 0x2d, 0x00, 0x00, 0x37, 0x34, 0x68, 0x59, 0xff, 0xa1]

def create_palette():
    out = []
    for i in range(len(reds)):
        out.append((reds[i], greens[i], blues[i]))
    return out

palette = create_palette()

def image_from_data_raw(data):
    out = Image.new("RGB", (SCREENSHOT_X, SCREENSHOT_Y))
    p = 0
    blue_pixels = 0.0

    for y in range(0, SCREENSHOT_Y):
        for x in range(0, SCREENSHOT_X):
            is_odd = ((x & 1) == 1)
            mask = 0x0f
            shift = 0
            if is_odd:
                mask = 0xf0
                shift = 4

            color = (ord(data[p]) & mask) >> shift
            rgb = palette[color]
            out.putpixel( (x, y), rgb )

            if color == 6:
                blue_pixels = blue_pixels + 1
            if is_odd:
                p = p + 1

    # Interestingness is defined as the inverse proportion of
    # blue pixels on the screen.
    interestingness = 1.0 - (blue_pixels / (SCREENSHOT_X * SCREENSHOT_Y))
    out.interestingness = interestingness

    return out

def image_from_data(data):
    out = Image.open(data)
    sz = out.size
    blue_pixels = 0.0

    for y in range(0, sz[1]):
        for x in range(0, sz[1]):
            pxl = out.getpixel( (x,y) )
            if pxl == palette[6]:
                blue_pixels = blue_pixels + 1

    interestingness = 1.0 - (blue_pixels / (sz[0] * sz[1]))
    out.interestingness = interestingness

    return out

def save_image(img, filename):
    img.save(filename)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "Usage: image.py datafile"
        sys.exit(1)
    name = sys.argv[1]

    f = open(name)
    data = f.read()
    f.close()
    img = image_from_data(data)
    save_image(img, name + ".png")

