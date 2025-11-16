#pragma once

namespace GameEngine {

	// 基底クラス
	class GameObject {
		virtual ~GameObject() = default;
	};

	// 受け渡すデータ
	struct UserData {
		// 基底クラスポインタ
		GameObject* object = nullptr;

		// 型安全なキャストヘルパー
		template<typename T>
		T* As() const {
			// キャストが間違えていればnullptrを返す
			return dynamic_cast<T*>(object);
		}

		// 有効化判断する
		bool IsActive() const { return object != nullptr; }
	};
}