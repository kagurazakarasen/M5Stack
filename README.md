# M5Stack関連のおもちゃ箱
M5Stack関係の試行錯誤などいろいろです。

## ENV_LOG
UNV UNIT（ https://www.switch-science.com/catalog/5690/ ）を使った環境データのロギング＆表示＆クラウドにアップロード

<img src="https://github.com/kagurazakarasen/M5Stack/blob/Garage/Garage/graph_img1.png" alt="サンプル" title="サンプル" width=320px>

こんなのを作りました。

詳しくは、
https://note.com/rasen/n/naa51e575b0e4
から始まる noteをご参照ください。

なお、ここに入れてある

* Adafruit_Sensor.h
* DHT12.cpp
* DHT12.h

は、

https://github.com/nishizumi-lab

の

https://github.com/nishizumi-lab/sample/tree/master/m5stack/bmp280/arduino/bmp280

からいただいています。

WiFiの設定は
https://qiita.com/kmaepu/items/c390d80973efa316ca4a
の方法をお借りしています。（若干ソースを書き換えています）

SDカード内に wifi.csv というファイルを作成し、

SSID,〈SSID〉  
PASS,〈PASS〉  

と言う内容でcsvファイルを作成しておくと、そのWiFi環境に接続します（デリミタは "," カンマ）

WiFi接続ができればNTPサーバに接続して日時設定をおこないます。

クラウドはSlackにポストする形です。

https://blog.nakajix.jp/entry/2016/02/08/090000#f-14455e8d

を参考に、「Webhook URL」を取得。

SDカード内に
slackhook.txt  を作成、取得したWebhook URLをそのまま記入します。

`https://hooks.slack.com/services/XXXXXX/XXXXXX/XXXXXXXXXXX`

↑改行は入れないでください。


---



### そのほか参考：

M5Stack Library
https://github.com/m5stack/M5Stack/blob/master/docs/getting_started_ja.md

LCDまわり
https://github.com/m5stack/m5-docs/blob/master/docs/ja/api/lcd.md


Arduino 日本語リファレンス
http://www.musashinodenpa.com/arduino/ref/index.php



---

### M5Stack関連の記事ナド

https://note.com/rasen/m/m2bfb6db23200

---

### ディレクトリ構成について

基本、M5_hogehoge と、アンダーバーでつながっているディレクトリ内には、hogehogeというユニット（センサ）を M5Stackにくっつけてつかうものです。

M5が頭についていないディレクトリ (ENVLOGナド) は、その名前のプロジェクト用のディレクトリ。

M5StickCナドの名前になっているディレクトリは、本体をM5StickCにして、下位ディレクトリにそれぞれのプロジェクトをいれてあります。

---

もちろん、どのプログラム・スケッチについても無保証です。（ハードもライブラリもがんがん進化してしまうので、古い奴はそのままではうごかないかも？）
