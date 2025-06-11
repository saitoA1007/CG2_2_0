#include"AudioManager.h"
#include"EngineSource/Common/ConvertString.h"
#include<cassert>
#include <filesystem>

#pragma comment(lib,"xaudio2.lib")

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

using namespace GameEngine;

AudioManager::~AudioManager() {}

void AudioManager::Finalize() {
	xAudio2_.Reset();
	SoundUnload();

	MFShutdown();

	CoUninitialize();
}

void AudioManager::Initialize() {

	// MediaFoundationの初期化
	MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);

	// XAudioエンジンのインスタンスを生成
	HRESULT result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	// マスターボイスを生成
	result = xAudio2_->CreateMasteringVoice(&masterVoice_);
	assert(SUCCEEDED(result));
}

void AudioManager::SoundPlayWave(const SoundData& soundData) {
	HRESULT result;

	// 波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}

AudioManager::SoundData AudioManager::SoundLoadWave(const std::string& filename) {
	
	// ファイル入力ストリームのインスタンス
	std::ifstream file;
	// .wavファイルをバイナリモードで開く
	file.open(filename, std::ios_base::binary);
	// ファイルオープン失敗を検出する
	assert(file.is_open());

	// RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	// タイプがWAVRかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	FormatChunk format = {};
	ChunkHeader chunk = {};

	// チャンクを順に読みながらfmtチャンクを探す
	while (file.read((char*)&chunk, sizeof(chunk))) {
		if (strncmp(chunk.id, "fmt ", 4) == 0) {
			// サイズが適正か確認
			assert(chunk.size <= sizeof(format.fmt));
			format.chunk = chunk;
			file.read((char*)&format.fmt, chunk.size);
			break;
		} else {
			// 必要のないチャンクは読み飛ばす
			file.seekg(chunk.size, std::ios_base::cur);
		}
	}

	// fmtチャンクが見つからなければエラー
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	// Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	// JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		// 読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		// 再読み込み
		file.read((char*)&data, sizeof(data));

	}

	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}
	// Dataチャンクのデータ部（波形データ）の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);
	// Waveファイルを閉じる
	file.close();

	// returnする為の音声データ
	SoundData soundData = {};
	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;
	soundData.name = filename;
	soundData.type = WAV;

	return soundData;
}

void AudioManager::SoundUnload() {

	for (uint32_t i = 0; i < soundData_.size(); ++i) {
		// バッファのメモリを解放
		delete[] soundData_[i].pBuffer;

		soundData_[i].pBuffer = 0;
		soundData_[i].bufferSize = 0;
		soundData_[i].wfex = {};
	}
}

uint32_t AudioManager::Load(const std::string& fileName) {

	// データ数が0でなく、同じ音声データがあればその配列番号を返す
	if (soundData_.size() != 0) {
		for (uint32_t i = 0; i < soundData_.size(); ++i) {
			if (soundData_.at(i).name == fileName) {
				return i;
			}
		}
	}

	// 拡張子からファイルタイプを判別
	std::filesystem::path path(fileName);
	std::string extension = path.extension().string();

	// 各拡張子をロード
	if (extension == ".wav") {
		soundData_.push_back(SoundLoadWave(fileName));
	} else if (extension == ".mp3") {
		soundData_.push_back(SoundLoadMp3(ConvertString(fileName)));
	} else {
		// サポートしていない拡張子への対応
		throw std::runtime_error("Unsupported audio format: " + extension);
	}

	// 読み込んだデータが格納されている配列番号を返す
	return uint32_t(soundData_.size() - 1);
}

AudioManager::SoundData AudioManager::SoundLoadMp3(const std::wstring path) {

	SoundData soundData = {};

	// 音声データを登録
	soundData.type = MP3;
	soundData.name = ConvertString(path);

	// ソースリーダーの作成
	IMFSourceReader* pMFSourceReader{ nullptr };
	MFCreateSourceReaderFromURL(path.c_str(), NULL, &pMFSourceReader);

	// メディアタイプを取得
	IMFMediaType* pMFMediaType{ nullptr };
	MFCreateMediaType(&pMFMediaType);
	pMFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pMFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	pMFSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pMFMediaType);

	pMFMediaType->Release();
	pMFMediaType = nullptr;
	pMFSourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pMFMediaType);

	WAVEFORMATEX* waveFormat{ nullptr };
	MFCreateWaveFormatExFromMFMediaType(pMFMediaType, &waveFormat, nullptr);

	// データ読み込み
	std::vector<BYTE> bufferData;
	while (true) {
		IMFSample* pMFSample{ nullptr };
		DWORD dwStreamFlags = 0;
		pMFSourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &dwStreamFlags, nullptr, &pMFSample);

		if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
			break;
		}

		IMFMediaBuffer* pMFMediaBuffer{ nullptr };
		pMFSample->ConvertToContiguousBuffer(&pMFMediaBuffer);

		BYTE* pBuffer = nullptr;
		DWORD cbCurrentLength = 0;
		pMFMediaBuffer->Lock(&pBuffer, nullptr, &cbCurrentLength);
		// データを一時バッファに追加
		bufferData.insert(bufferData.end(), pBuffer, pBuffer + cbCurrentLength);

		// 解放処理
		pMFMediaBuffer->Unlock();
		pMFMediaBuffer->Release();
		pMFSample->Release();

	}

	// 読み込んだデータをメモリにコピー
	BYTE* pAudioData = new BYTE[bufferData.size()];
	memcpy(pAudioData, bufferData.data(), bufferData.size());

	soundData.wfex = *waveFormat;
	soundData.pBuffer = pAudioData;
	soundData.bufferSize = static_cast<UINT32>(bufferData.size());

	// 解放処理
	CoTaskMemFree(waveFormat);
	pMFMediaType->Release();
	pMFSourceReader->Release();

	return soundData;
}

void AudioManager::Play(uint32_t soundHandle) {

	// 音声を再生
	if (soundData_[soundHandle].type == MP3) {
		SoundPlayMp3(soundData_[soundHandle]);
	} else {
		SoundPlayWave(soundData_[soundHandle]);
	}
}

void AudioManager::SoundPlayMp3(const SoundData& soundData) {
	HRESULT result;

	IXAudio2SourceVoice* pSourceVoice{ nullptr };
	xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);

	XAUDIO2_BUFFER buffer{ 0 };
	buffer.pAudioData = soundData.pBuffer;
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.AudioBytes = sizeof(BYTE) * static_cast<UINT32>(soundData.bufferSize);
	
	// 再生する
	result = pSourceVoice->SubmitSourceBuffer(&buffer);
	result = pSourceVoice->Start();
}