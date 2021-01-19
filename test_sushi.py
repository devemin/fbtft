from PIL import Image, ImageDraw, ImageFont, ImageFilter


def color565(r, g=0, b=0):
    try:
        r, g, b = r
    except TypeError:
        pass
    return (r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3


width=640
height=48
rotation = 0


fbbuffer = bytearray(640*48*2)


imsushi = Image.open('sushi.jpg')
image = Image.new("RGB", (width, height), (255,255,255))
draw = ImageDraw.Draw(image)




posx = 0


f = open('/dev/fb1', 'wb')

while(1):
    draw_im = image.copy()
    draw_im.paste(imsushi, (posx +96*0, 0))
    draw_im.paste(imsushi, (posx +96*1, 0))
    draw_im.paste(imsushi, (posx +96*2, 0))
    draw_im.paste(imsushi, (posx +96*3, 0))
    draw_im.paste(imsushi, (posx +96*4, 0))
    draw_im.paste(imsushi, (posx +96*5, 0))

    imgdata = draw_im.getdata()

    for y in range(height):
        for x in range(width):
            testdata = color565(imgdata[y*width+x])
            fbbuffer[(y*width+x)*2    ] = testdata & 0x0F
            fbbuffer[(y*width+x)*2 +1 ] = testdata >> 8
    data = f.write(fbbuffer)
    f.seek(0)
    posx += 6
    if (posx >= 96):
        posx = 0


f.close()
