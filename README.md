# cit_robosys2020
CITロボットシステム学 講義用リポジトリ

## 実行とログ確認
カーネルモジュールのインストール

1. `myled/`下で`make`
1. `sudo insmod myled.ko`
1. `sudo chmod 666 /dev/myled`

デバイスに文字列`'1'`を渡す
```
echo 1 > /dev/myled
```
デバイスが`copy_to_user`している文字を読む
```
cat /dev/myled
```
デバイスが`printk`したログを閲覧する
```
tail /var/log/kern.log
```

## 引っかかったところ
以下エラーが出る。
```
myled.c:75:14: error: implicit declaration of function ‘ioremap_nocache’; did you mean ‘ioremap_cache’? [-Werror=implicit-function-declaration]
   75 |  gpio_base = ioremap_nocache(RPI_REG_BASE, 0xA0);
      |              ^~~~~~~~~~~~~~~
      |              ioremap_cache
```
カーネルが古いのかと思ったがそうではない模様。
以下RasPi上で`uname -r`した結果。
```
5.8.0-1007-raspi
```
仕方が無いので`ioremap()`でとりあえず進めた。

