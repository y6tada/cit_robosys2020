# cit_robosys2020
CITロボットシステム学 講義用リポジトリ  
Original lecture: [ryuichiueda/robosys2020](https://github.com/ryuichiueda/robosys2020)

## 説明
家電の遠隔操作に使われるIRリモコンの信号読み取りと吐き出しをして、家電のリモコン操作をRasPiから行えるようにする。
IRリモコンの信号は38〜40kHzで変調された信号にパルスの信号をかぶせてあるので、PWMで変調を行い、信号はPWMの吐き出しタイミングを制御すること行う。

### マイコンの処理
マイコンで行われるのは以下のシンプルな順序処理、詳細は`FRDM_K64F/main.cpp`を参照。

1. IR信号を検出するまで待機
1. IR信号を検出したら、50us周期のサンプリングで`MAX_MEASURING`回数だけピン状態をバッファする
1. IR信号の読み込みが完了したら、ユーザーがSW2を押すかRasPiがGPIOを上げるまで待機
1. ユーザかSW２を押すかRasPiがGPIOを上げたら、記憶したIR信号をIR-LEDから出力

IR信号の読み込み結果は処理過程でシリアルプリントされる。
以下は横軸をmsに取ったIR信号。
本当なら会社ごとにきちんとエンコード形式があるらしく、それを理解してやれば要素数3000のゼロイチ配列ではなく、数バイトの文字列だけでこの信号は作れるらしい。
今回は、そこは重要ではないと判断してゼロイチ記憶のゼロイチ書き出しで対応。	

![ir_signal_log](https://user-images.githubusercontent.com/18658190/101500087-5865e700-39b1-11eb-8f58-01ad96c528a1.png)

### オリジナルから変更が加えられた箇所
講義で扱われたドライバから加えられた変更は以下の2つのみ。
最初はRasPiのGPIOをタイマーベースで制御し、数usオーダーのピン操作が可能なのか調べたが、どうやら厳しいという結論に至る。

1. myledモジュールに`p = pulse`のモードを追加
1. タイマーでGPIOを落とす(200ms待機)

### 

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
### 関数`ioremap_nocache()`がない
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
仕方が無いので`ioremap()`でとりあえず進めた。i
### `linux/hrtimer`のタイマーハンドラの戻り値
`myled.c`の`static enum hrtimer_restart timer_handler(struct hrtimer *_timer)`は以下で定義されるenumの戻り値を返すが、これに引数を指定しないOR`HRTIMER_RESTART`を返すと暴走する。
```
enum hrtimer_restart {
	HRTIMER_NORESTART,	/* Timer is not restarted */
	HRTIMER_RESTART,	/* Timer must be restarted */
};
```


