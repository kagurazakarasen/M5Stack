# M5Stack関連のおもちゃ箱
M5Stack関係の試行錯誤などいろいろです。

## ENV_LOG
UNV UNITを使った環境データのロギング

<img src="https://github.com/kagurazakarasen/M5Stack/blob/Garage/Garage/graph_img1.png" alt="サンプル" title="サンプル" width=320px>

こんなのを作りました。

詳しくは、
https://note.com/rasen/n/naa51e575b0e4
をから始まる noteをご参照ください。

なお、ここに入れてある

* Adafruit_Sensor.h
* DHT12.cpp
* DHT12.h

は、

https://github.com/nishizumi-lab

の

https://github.com/nishizumi-lab/sample/tree/master/m5stack/bmp280/arduino/bmp280

からいただいています。



そのほか参考：

M5Stack Library
https://github.com/m5stack/M5Stack/blob/master/docs/getting_started_ja.md

LCDまわり
https://github.com/m5stack/m5-docs/blob/master/docs/ja/api/lcd.md

※ M5.Lcd.drawRect(0, 110, 320, 240, G_FLAME_COLOR); //なぜか下のラインが出ない　※239にしても220や200にしても出ず。バグかな？


Arduino 日本語リファレンス
http://www.musashinodenpa.com/arduino/ref/index.php
