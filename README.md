## Doom8088: X68000 Edition
![Doom8088: X68000 Edition](readme_imgs/DOOMX68K.png?raw=true)

[Doom8088ST](https://github.com/FrenkelS/Doom8088ST)をベースにX68000に移植した物です。X68030やそれに準じたアクセラレータが無くともMC68000、最低でもメモリ2MBを搭載したX68000で動きます。

ただしMC68000 10MHzでは重いためX68000 XVI(MC68000 16MHz)以上・またはアクセラレータ搭載機を推奨、空きメモリ4MBまでヒープ用メモリを取得するためメモリ6MB以上搭載していればその分ゲーム中の読み込みが軽減され動作が軽くなります。

**仕様**
 - Doom 1 エピソード1のみプレイ可能
 - 回転表示されるマップ
 - demo3のみ再生可能
 - 256色表示
 - BEEP音による効果音をFM音源で疑似的に再現
 - 音楽無し
 - 床や天井のテクスチャマッピングは非対応
 - シェーディング表示はなし
 - セーブ・ロード機能なし
 - マルチプレイヤーなし
 - PWAD使用不可
 - 画面サイズの変更不可
 - マウスやジョイスティックには非対応

## 操作方法:
|Action                 |X68000                |
|-----------------------|----------------------|
|攻撃 / キャンセル       |CTRL                  |
|使用 / 決定             |スペース or ENTER     |
|走り                   |SHIFT                 |
|移動                   |矢印キー               |
|横移動                  |XF3                   |
|左右横移動              |XF1 & XF2   or , & .    |
|オートマップ            |TAB                   |
|オートマップズームアウト |+ & -                 |
|オートマップモード切替   |F                     |
|武器変更                |OP.1 & OP.2   or ] & [  |
|Menu                   |Esc                   |

## チート:
|コード     |効果                     |メモ                            |
|----------|-------------------------|--------------------------------|
|IDCHOPPERS|Chainsaw                 |                                |
|IDDQD     |God mode                 |                                |
|IDKFA     |Weapons & Keys           |                                |
|IDFA      |Weapons                  |                                |
|IDSPISPOPD|No Clipping              |                                |
|IDBEHOLDV |Invincibility            |                                |
|IDBEHOLDS |Berserk                  |                                |
|IDBEHOLDI |Invisibility             |                                |
|IDBEHOLDR |Radiation shielding suit |                                |
|IDBEHOLDA |Auto-map                 |                                |
|IDBEHOLDL |Lite-Amp Goggles         |                                |
|IDCLEV    |Exit Level               |                                |
|IDEND     |Show end text            |                                |
|IDROCKET  |Enemy Rockets            |(GoldenEye)                     |
|IDRATE    |Toggle FPS counter       |Divide by 10 to get the real FPS|

## コマンドラインオプション:
|コマンドライン        |効果                 |
|---------------------|---------------------|
|`-nosfx`             |効果音なし            |
|`-nosound`           |効果音なし            |
|`-timedemo demo3`    |ベンチマーク起動      |


## 謝辞
コンパイルにはSHARP X680x0 シリーズ対応のクロスコンパイル環境[xdev68k](https://github.com/yosshin4004/xdev68k)を利用させて頂きました。

X68000プログラミングでのCRTC・VRAM操作に[x68000 DooM Code](https://sourceforge.net/p/x68000-doom/code/ci/master/tree/)を参考にさせて頂きました。
