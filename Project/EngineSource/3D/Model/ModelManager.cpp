#include"ModelManager.h"

using namespace GameEngine;

ModelManager::~ModelManager() {
	models_.clear();
}

void ModelManager::RegisterMode(const std::string& modelName, const std::string& modelFile) {

	// 同名のモデルが登録されている場合は早期リターン
	auto getName = nameToHandles_.find(modelName);
	if (getName != nameToHandles_.end()) {
		return;
	}

	// 新しいハンドルを取得
	uint32_t handle = nextHandle_++;

	// 登録データするを作成
	ModelEntryData entryData;
	entryData.name = modelName;
	entryData.model = Model::CreateModel(modelName, modelFile);

	// 登録する
	models_[handle] = std::move(entryData);
	nameToHandles_[modelName] = handle;
}

void ModelManager::RegisterMode(const std::string& modelName, std::unique_ptr<Model> model) {
	// 同名のモデルが登録されている場合は早期リターン
	auto getName = nameToHandles_.find(modelName);
	if (getName != nameToHandles_.end()) {
		return;
	}

	// 新しいハンドルを取得
	uint32_t handle = nextHandle_++;

	// 登録データするを作成
	ModelEntryData entryData;
	entryData.name = modelName;
	entryData.model = std::move(model);

	// 登録する
	models_[handle] = std::move(entryData);
	nameToHandles_[modelName] = handle;
}

void ModelManager::UnregisterModel(uint32_t handle) {
	auto getModel = models_.find(handle);
	if (getModel == models_.end()) {
		return;
	}

	// 名前からマップを削除
	const std::string& name = getModel->second.name;
	nameToHandles_.erase(name);

	// 登録したモデル本体を削除
	models_.erase(getModel);
}

uint32_t ModelManager::GetHandleByName(const std::string& name) const {
	auto getHandle = nameToHandles_.find(name);
	if (getHandle == nameToHandles_.end()) {
		return 0;
	}
	return getHandle->second;
}

std::string ModelManager::GetNameByHandle(uint32_t handle) const {
	auto getName = models_.find(handle);
	if (getName == models_.end()) {
		return "";
	}
	return getName->second.name;
}

[[nodiscard]]
Model* ModelManager::GetHandleByModel(uint32_t handle) const {
	auto getModel = models_.find(handle);
	if (getModel == models_.end()) {
		return nullptr;
	}

	if (getModel->second.model) {
		return getModel->second.model.get();
	}

	return nullptr;
}

[[nodiscard]]
Model* ModelManager::GetNameByModel(const std::string& name) const {

	auto getHandle = nameToHandles_.find(name);
	if (getHandle == nameToHandles_.end()) {
		return nullptr;
	}

	auto getModel = models_.find(getHandle->second);
	if (getModel == models_.end()) {
		return nullptr;
	}

	if (getModel->second.model) {
		return getModel->second.model.get();
	}

	return nullptr;
}