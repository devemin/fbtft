#for maji majo Iris LCD

マジマジョアイリス用のmod です。

（ライセンス関係まだチェックしてないです・・・）


#HOWTO
```
sudo apt-get install bc bison flex libssl-dev libncurses5-dev
sudo wget https://raw.githubusercontent.com/notro/rpi-source/master/rpi-source -O /usr/bin/rpi-source
sudo chmod +x /usr/bin/rpi-source
sudo /usr/bin/rpi-source -q --tag-update
sudo rpi-source
```

/root/内にLinux カーネルソース一式が準備されます。

また、/lib/modules/x.x.x.../build にシンボリックリンクが作成されます。

以上で、カーネルモジュールをビルドする環境が1クリックで作られたことになります。


参考
https://qiita.com/RCA3610/items/02d8274d78ee8c26e8c9


#マジョカアイリスLCD接続


#ドライバをビルド

今回、fb_s6d1121.c　というファイルをマジョカアイリス液晶用に改造させてもらいました。（パラメータベタ書き許して・・・）


```
sudo make -j4
```


```
#ドライバをロード

./majiinsmod.sh

#gpioの接続が正しければ、液晶が暗転します。

#ドライバをアンロード

./majirmmod.sh
```

#フレームバッファ使い方

https://github.com/notro/fbtft/wiki/Framebuffer-use

fbcpでデスクトップ写すのが面白いですね。　あとは fbi 等

（con2fbmap はsudo raspi-config のブートのところをコンソールブートにしないと、GUIブートだとできなそう？というかできなかった）

https://qiita.com/kitazaki/items/9f6119d7dc21cd29268e

fbtest　（描画テスト）

このあたりのページが参考になります。


```
git clone https://git.kernel.org/pub/scm/linux/kernel/git/geert/fbtest.git
cd fbtest
make
./fbtest --fbdev /dev/fb1
```


#補足

```
#fbtft関連のドライバの読み込み状況確認
lsmod | grep fb

#ドライバ１個ずつロード
sudo insmod xxx
#ドライバ１個ずつアンロード
sudo rmmod xxx

```



以下、original README

  FBTFT
=========

2015-01-19
The FBTFT drivers are now in the Linux kernel staging tree: https://git.kernel.org/cgit/linux/kernel/git/gregkh/staging.git/tree/drivers/staging/fbtft?h=staging-testing
Development in this github repo has ceased.


Linux Framebuffer drivers for small TFT LCD display modules.
The module 'fbtft' makes writing drivers for some of these displays very easy.

Development is done on a Raspberry Pi running the Raspbian "wheezy" distribution.

INSTALLATION
  Download kernel sources

  From Linux 3.15  
    cd drivers/video/fbdev
    git clone https://github.com/notro/fbtft.git
    
    Add to drivers/video/fbdev/Kconfig:   source "drivers/video/fbdev/fbtft/Kconfig"
    Add to drivers/video/fbdev/Makefile:  obj-y += fbtft/
  
  Before Linux 3.15  
    cd drivers/video
    git clone https://github.com/notro/fbtft.git
    
    Add to drivers/video/Kconfig:   source "drivers/video/fbtft/Kconfig"
    Add to drivers/video/Makefile:  obj-y += fbtft/
  
  Enable driver(s) in menuconfig and build the kernel


See wiki for more information: https://github.com/notro/fbtft/wiki


Source: https://github.com/notro/fbtft/
