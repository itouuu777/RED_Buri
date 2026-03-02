なぜうまくいったのでしょうか


原因は2点です。

USART1 PA9 のGPIOモード差
AF_OD から、動作実績側と同じ AF_PP に合わせたことで物理層が安定しました。

getPosition() の受信処理
toRx 直後の HAL_Delay(1)/再クリアで先頭バイトを落とす可能性がありました。
これをやめて、まず8バイト固定応答を即受信する形にしたことで、位置応答を正しく拾えるようになりました。

結果として、今は startPos=3638 comm=1 になっており、通信も位置取得も正常です。


具体的にはどのファイルを変更したのですか？



変更したのはこの4ファイルです。

Src/user_run.cpp
lib/sts3215/sts3215.cpp
Src/usart.c
nucleo_f446re.ioc


usartとiocのどの部分いじりました？


USART1 の PA9 設定だけです。

Src/usart.c
HAL_UART_MspInit() の USART1 GPIO Configuration で
GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
→ GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;

nucleo_f446re.ioc
PA9 の設定に以下を追加して AF_PP を明示
PA9.GPIOParameters=GPIO_PuPd,GPIO_ModeDefaultOD
PA9.GPIO_ModeDefaultOD=GPIO_MODE_AF_PP
