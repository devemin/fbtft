from PIL import Image, ImageDraw, ImageFont, ImageFilter
import cv2





def color565(r, g=0, b=0):
    try:
        r, g, b = r
    except TypeError:
        pass
    return (r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3



width=640
height=48
rotation = 0

#縮小する幅サイズ
scale_width = 48

fbbuffer = bytearray(640*48*2)

background = Image.new("RGB", (width, height), (255,255,255))

# VideoCapture オブジェクトを取得します
capture = cv2.VideoCapture(0)

f = open('/dev/fb1', 'wb')

scale_cnt = 0
scale_max = 300

while(True):
    scale_width_now = scale_width + scale_cnt

    ret, frame = capture.read()
    frame_copy = frame.copy()
    frame_copy = cv2.cvtColor(frame_copy, cv2.COLOR_BGR2RGB) # cv2の画像はBGRなのでRGBに変換
    camimage = Image.fromarray(frame_copy)
    img_resize = camimage.resize((scale_width_now, height))

    bg_copy = background.copy()
    bg_copy.paste(img_resize, (int(width/2)-int(scale_width_now/2), 0))

    draw = ImageDraw.Draw(bg_copy)

    imgdata = bg_copy.getdata()

    for y in range(height):
        for x in range(width):
            testdata = color565(imgdata[y*width+x])
            fbbuffer[(y*width+x)*2    ] = testdata & 0x0F
            fbbuffer[(y*width+x)*2 +1 ] = testdata >> 8
    data = f.write(fbbuffer)
    f.seek(0)


    cv2.imshow('frame',frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

    scale_cnt = scale_cnt + 10
    if (scale_cnt >= scale_max):
        scale_cnt = 0

f.close()


capture.release()
cv2.destroyAllWindows()

