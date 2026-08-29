# winter02

## 1. 概要

このプログラムは、OpenGL と GLSL を用いてボーンの階層構造（スケルトン）を定義し、ボーンの姿勢変化に応じた頂点ブレンディング（バーテックスブレンディング／スキニング）の基礎を学ぶためのサンプルプログラムです。本プログラムは、以下のブログ記事の解説に対応しています。

- [第１６回 バーテックスブレンディング](https://tokoik.github.io/blog/今風%20opengl%20の使い方/2009/12/31/glsl.html)

本雛形プログラムでは、2本のボーンの階層構造と円柱状に散布された点群が配置されており、マウスドラッグによってボーンのアニメーション変形を行うことができます。

## 2. 対応環境

- **Windows**: Visual Studio 2019 以降 / CMake 3.22 以降
- **macOS**: Xcode 12 以降 / CMake 3.22 以降
- **Linux (Ubuntu 等)**: GCC / Clang / CMake 3.22 以降

## 3. ビルド手順

### 3.1 Windows (Visual Studio)

```powershell
cmake -B build
cmake --build build --config Release
```

### 3.2 Linux (Ubuntu)

必要なパッケージをインストールした上でビルドします。

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev libglew-dev
cmake -B build
cmake --build build
```

### 3.3 macOS (Xcode)

```bash
cmake -B build -G Xcode
cmake --build build --config Release
```

## 4. 起動方法

ビルド完了後、生成された実行ファイルを実行します。

```powershell
./build/Release/winter02.exe
```

## 5. 操作方法

- **マウス左ドラッグ**: 根元のボーン（ボーン0）を回転します。
- **マウス右ドラッグ**: 先端のボーン（ボーン1）を回転します。
- **ウィンドウの閉じるボタン / コンソールで `Ctrl+C`**: プログラムを終了します。

## 6. プログラムの解説

### 6.1 ボーンクラス (`Bone`)

ボーンの初期配置（位置 `position`、回転 `rotation`、長さ `length`）と、各ボーンに対する変形アニメーション行列 `animation`、および親ボーンへのポインタ `parent` を保持します。

### 6.2 ボーンのアニメーションとマトリックスパレット

各ボーンの初期位置を求める変換行列 $\mathbf{M}_i$ とアニメーション後の変換行列 $\mathbf{B}_i(t)$ を根元から累積して求めます。初期位置からアニメーション後の位置へ移動する変換行列 $\mathbf{B}_i(t) \mathbf{M}_i^{-1}$ を計算して uniform 変数の配列（マトリックスパレット `blendMatrix`）としてシェーダに渡します。

### 6.3 応用: バーテックスブレンディングの実装 (`vertexblend.vert`)

ボーンと各頂点との最短距離 $d$ を計算し、重み $w_i = (d + 1)^{-c}$ を用いて各ボーンの変形結果を重み付け線形結合することで、ボーンの動きに追従した滑らかなメッシュ（点群）変形を実現できます。
