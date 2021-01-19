from PIL import Image, ImageDraw, ImageFont, ImageFilter
import cv2


width=640
height=48
rotation = 0

fbbuffer = bytearray(640*48*2)


# VideoCapture オブジェクトを取得します
capture = cv2.VideoCapture(0)

f = open('/dev/fb1', 'wb')

while(True):
    ret, frame = capture.read()
    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB) # cv2の画像はBGRなのでRGBに変換
    image = Image.fromarray(frame)
    img_resize = image.resize((width, height))
    draw = ImageDraw.Draw(img_resize)

    imgdata = img_resize.getdata()

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


f.close()


capture.release()
cv2.destroyAllWindows()

