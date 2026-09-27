
■ Loquaty github リポジトリ

https://github.com/leshade/loquaty



● Loquaty リリースファイルには以下のものが含まれています。

〇 64bit Windows 用コマンドラインツール
	- bin\win64\loquaty.exe
		Loquaty 実行ファイル

	- bin\win64\plugins
		サンプル用プラグイン DLL

〇 32bit Windows 用
	- bin\win32\loquaty.exe
		Loquaty 実行ファイル

	- bin\win32\plugins
		サンプル用プラグイン DLL

〇 Loquaty スクリプト用ライブラリ
	+ bin\library
		Loquaty 用インクルードファイル

〇 ドキュメント
	- doc\manual.xhtml
		Loquaty 言語マニュアル

	- doc\index.xhtml
		クラス一覧

〇 サンプルゲームスクリプト
	+ example\SimpleGame
		シンプルなシューティングゲーム
		実行は run.bat

	+ example\AvoidanceFlight
		シンプルなフライトゲーム
		実行は run.bat



● 実行に必要なファイル

　ビルド済みの exe ファイルを実行するには「Visual C++ 再頒布可能パッケージ」（Visual Studio 2022 を含むもの）をインストールしてください。

https://learn.microsoft.com/ja-jp/cpp/windows/latest-supported-vc-redist



● ライセンス

〇 Loquaty

Copyright (C) 2024-2025 Leshade Entis (理影).  
Apache License license.  
http://www.apache.org/licenses/


〇 tinygltf (entisgls4.dll に含まれる)

MIT License

Copyright (c) 2017 Syoyo Fujita, Aurélien Chatelain and many contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


〇 tinygltf で参照するサードパーティーライブラリ

json.hpp : Licensed under the MIT License http://opensource.org/licenses/MIT. Copyright (c) 2013-2017 Niels Lohmann http://nlohmann.me.

stb_image : Public domain.



● 更新履歴

2025/02/09  ver.1.01
デバッグ機能追加




