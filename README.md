# FeatureExtract_thinning
特徴強調可視化プログラム

## 使い方

### ・PointFeatureExtraction_v007
入力された点群の座標値に対して主成分分析を行い，特徴量を算出するプログラムです．  
読み込み可能なデータは以下の形式になります．
- ply
- xyz
- spbr

#### プログラムの使い方
1. ディレクトリに移動して以下のコマンドを入力してください．コマンドが実行されると，```pfe```という名前の実行ファイルが作成されます．
```
$ make
```
2. 入力ファイルと出力ファイルを記述してください．3つめの```[output_file_name.xyz]```はオプションです，書かなかった場合には```out.xyz```というファイル名で結果が出力されます．
```
$ ./pfe [input_file_name.ply or .xyz or .spbr] [output_file_name.xyz]
```

### ・alphaControl4PLY_withFeature_thinning
点群全体と特徴領域を半透明融合可視化するプログラムです．特徴量に応じて不透明度を制御します．

#### プログラムの使い方
1. ディレクトリに移動して以下のコマンドを入力してください．コマンドが実行されると，```alphaControl4ply```という名前の実行ファイルが作成されます．
```
$ make
```

2. 以下のコマンドを実行すると，```.spbr```ファイル(点群全体のデータ)と```.spbr_f.spbr```(特徴領域のデータ)が出力されます．
```alphaControl4ply```には以下のオプションがあります．

- -l
- -a
- -i
- -fa
- -ft

```
$ ./alphaControl4ply [input_file_name.xyz] -a [全体の不透明度] -ft [特徴領域の閾値]
```
