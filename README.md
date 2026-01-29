int Novice::GetTextureSize(int textureHandle, int* width, int* height) {
	if (!width || !height) {
		return 0; // nullptrチェック
	}

	if (textureHandle < 0) {
		*width = 0;
		*height = 0;
		return 0; // 無効なハンドル
	}

	try {
		const D3D12_RESOURCE_DESC& resourceDesc = TextureManager::GetInstance()->GetResoureDesc(textureHandle);

		*width = static_cast<int>(resourceDesc.Width);
		*height = static_cast<int>(resourceDesc.Height);
		return 1; // 成功
	} catch (...) {
		*width = 0;
		*height = 0;
		return 0; // エラー
	}
}

/// <summary>
/// テクスチャのサイズを取得
/// </summary>
/// <param name="textureHandle">LoadTexture()で取得したハンドル</param>
/// <param name="width">幅の格納先ポインタ</param>
/// <param name="height">高さの格納先ポインタ</param>
/// <returns>成功時1、失敗時0</returns>
static int GetTextureSize(int textureHandle, int* width, int* height);
