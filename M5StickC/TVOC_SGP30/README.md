# M5StickC用に最小構成でeCO2を計れるようにしたもの

## つかうもの

* M5Stick-C
https://www.switch-science.com/catalog/6350/
* TVOC/eCO2 ガスセンサユニット（SGP30）
https://www.switch-science.com/catalog/6619/
もともとM5Stack用のモジュール。これをM5StickCで流用します。

両者をGroveケーブルでつなぎ、このディレクトリのスケッチをArduinoIDEでコンパイルしてM5Stick-Cに送り込めば動作するはずです。

※WiFiとAmbientを使う場合は、MyConfig.h内を適宜かきかえてください
