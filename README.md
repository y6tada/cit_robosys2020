# cit_robosys2020
#CITロボットシステム学 講義用リポジトリ

## 引っかかったところ
以下エラーが出る
```
myled.c:75:14: error: implicit declaration of function ‘ioremap_nocache’; did you mean ‘ioremap_cache’? [-Werror=implicit-function-declaration]
   75 |  gpio_base = ioremap_nocache(RPI_REG_BASE, 0xA0);
      |              ^~~~~~~~~~~~~~~
      |              ioremap_cache
```
カーネルが古いのかと思ったがそうではない模様、以下RasPi上で`uname -r`した結果
```
5.8.0-1007-raspi
```
仕方が無いので`ioremap_cache`でとりあえず進めた。

