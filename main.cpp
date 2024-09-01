#include "Audio.h"
#include "AxisIndicator.h"
#include "DirectXCommon.h"
#include "GameScene.h"
#include "ImGuiManager.h"
#include "PrimitiveDrawer.h"
#include "TextureManager.h"
#include "TitleScene.h"
#include "WinApp.h"
#include"GameClear.h"

// 現在のシーンを保持するポインタ
GameScene* gameScene = nullptr;
TitleScene* titleScene = nullptr;
GameClear* clearScene = nullptr;

// シーンの種類を定義する列挙型
enum class Scene {
	kUnknown = 0, // 不明なシーン
	kTitle,       // タイトルシーン
	kGame,        // ゲームシーン
	kClear,
};

// 現在のシーンを示す変数
Scene scene = Scene::kTitle;

// シーンを切り替える関数
void ChangeScene() {
	switch (scene) {
	case Scene::kTitle:
		// タイトルシーンが終了したらゲームシーンに切り替える
		if (titleScene->IsFinished()) {
			scene = Scene::kGame;
			delete titleScene;         // タイトルシーンのメモリを解放
			titleScene = nullptr;      // ポインタをリセット
			gameScene = new GameScene; // 新しいゲームシーンを生成
			gameScene->Initialize();   // ゲームシーンの初期化
		}
		break;
	case Scene::kGame:
		// ゲームシーンが終了したらタイトルシーンに切り替える
		if (gameScene->IsFinished()) {
			scene = Scene::kClear;
			delete gameScene;            // ゲームシーンのメモリを解放
			gameScene = nullptr;         // ポインタをリセット
			clearScene = new GameClear; // 新しいタイトルシーンを生成
			clearScene->Initialize();    // タイトルシーンの初期化
		}
		break;

	case Scene::kClear:
		if (clearScene->IsFinished()) {
			scene = Scene::kTitle;
			delete clearScene;           // ゲームシーンのメモリを解放
			clearScene = nullptr;        // ポインタをリセット
			titleScene = new TitleScene; // 新しいタイトルシーンを生成
			titleScene->Initialize();    // タイトルシーンの初期化
		}
		break;


	}
}

// シーンの更新処理を行う関数
void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Update(); // タイトルシーンの更新処理
		break;
	case Scene::kGame:
		gameScene->Update(); // ゲームシーンの更新処理
		break;
	case Scene::kClear:
		clearScene->Update(); // クリアシーンの更新処理
		break;
	}
}

// シーンの描画処理を行う関数
void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw(); // タイトルシーンの描画処理
		break;
	case Scene::kGame:
		gameScene->Draw(); // ゲームシーンの描画処理
		break;
	case Scene::kClear:
		clearScene->Draw(); // クリアシーンの描画処理
		break;
	}
}

// Windowsアプリケーションのエントリーポイント (main 関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	WinApp* win = nullptr;
	DirectXCommon* dxCommon = nullptr;
	// 汎用機能のインスタンス
	Input* input = nullptr;
	Audio* audio = nullptr;
	AxisIndicator* axisIndicator = nullptr;
	PrimitiveDrawer* primitiveDrawer = nullptr;

	// ゲームウィンドウの作成
	win = WinApp::GetInstance();
	win->CreateGameWindow(L"LE2D_18_ハギワラ_ケンタ");

	// DirectX初期化処理
	dxCommon = DirectXCommon::GetInstance();
	dxCommon->Initialize(win);

#pragma region 汎用機能初期化
	// ImGuiの初期化
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();
	imguiManager->Initialize(win, dxCommon);

	// 入力デバイスの初期化
	input = Input::GetInstance();
	input->Initialize();

	// オーディオシステムの初期化
	audio = Audio::GetInstance();
	audio->Initialize();

	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon->GetDevice());
	TextureManager::Load("white1x1.png");

	// スプライトの静的初期化
	Sprite::StaticInitialize(dxCommon->GetDevice(), WinApp::kWindowWidth, WinApp::kWindowHeight);

	// 3Dモデルの静的初期化
	Model::StaticInitialize();

	// 軸方向表示の初期化
	axisIndicator = AxisIndicator::GetInstance();
	axisIndicator->Initialize();

	// プリミティブ描画機能の初期化
	primitiveDrawer = PrimitiveDrawer::GetInstance();
	primitiveDrawer->Initialize();
#pragma endregion

	// 最初にタイトルシーンを設定
	scene = Scene::kTitle;
	titleScene = new TitleScene;
	titleScene->Initialize();

	// メインループ
	while (true) {
		// メッセージ処理
		if (win->ProcessMessage()) {
			break;
		}

		// ImGuiの入力受付開始
		imguiManager->Begin();
		// 入力の毎フレーム更新処理
		input->Update();
		// シーンの切り替えと更新
		ChangeScene();
		UpdateScene();

		// 軸方向表示の更新
		axisIndicator->Update();
		// ImGuiの入力受付終了
		imguiManager->End();

		// 描画処理の開始
		dxCommon->PreDraw();
		// 現在のシーンを描画
		DrawScene();

		// 軸方向表示の描画
		axisIndicator->Draw();
		// プリミティブ描画のリセット
		primitiveDrawer->Reset();
		// ImGuiの描画処理
		imguiManager->Draw();
		// 描画処理の終了
		dxCommon->PostDraw();
	}

	// 各種解放処理
	delete titleScene;
	delete gameScene;
	// 3Dモデル関連のリソース解放
	Model::StaticFinalize();
	// オーディオの解放
	audio->Finalize();
	// ImGuiの解放
	imguiManager->Finalize();

	// ゲームウィンドウの破棄
	win->TerminateGameWindow();

	return 0;
}
