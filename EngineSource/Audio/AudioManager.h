#pragma once
#include<xaudio2.h>
#include<fstream>
#include <wrl.h>
#include<vector>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

namespace GameEngine {

	class AudioManager {
	public:
		// 拡張子の種類
		enum Type {
			MP3,
			WAV,
		};

		// チャンクヘッダ
		struct ChunkHeader {
			char id[4];   // チャンク毎のID
			int32_t size; // チャンクサイズ
		};

		// RIFFヘッダチャンク
		struct RiffHeader {
			ChunkHeader chunk; // RIFF
			char type[4];  // WAVE
		};

		// FMTチャンク
		struct FormatChunk {
			ChunkHeader chunk; // fmt
			WAVEFORMATEX fmt;  // 波形フォーマット
		};

		// 音声データ
		struct SoundData {
			// 波形フォーマット
			WAVEFORMATEX wfex;
			// バッファの先頭アドレス
			BYTE* pBuffer;
			// バッファのサイズ
			unsigned int bufferSize;
			// 音声データの名前
			std::string name;
			// 拡張子の種類
			Type type;
		};

	public:

		~AudioManager();

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize();

		/// <summary>
		/// 終了処理
		/// </summary>
		void Finalize();

		/// <summary>
		/// 音声をロード
		/// </summary>
		/// <param name="fileName"></param>
		/// <returns></returns>
		uint32_t Load(const std::string& fileName);

		/// <summary>
		/// 音声を再生
		/// </summary>
		/// <param name="soundHandle"></param>
		void Play(uint32_t soundHandle);

	private:

		Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
		IXAudio2MasteringVoice* masterVoice_;

		std::vector<SoundData> soundData_;

	private:
		/// <summary>
		/// .wav音声を再生
		/// </summary>
		/// <param name="xAudio2"></param>
		/// <param name="soundData"></param>
		void SoundPlayWave(const SoundData& soundData);

		/// <summary>
		/// .mp3音声を再生
		/// </summary>
		/// <param name="soundData"></param>
		void SoundPlayMp3(const SoundData& soundData);

		/// <summary>
		/// .wavファイルの読み込み
		/// </summary>
		/// <param name="filename"></param>
		/// <returns></returns>
		SoundData SoundLoadWave(const std::string& filename);

		/// <summary>
		/// .mp3ファイルの読み込み
		/// </summary>
		/// <param name="path"></param>
		/// <returns></returns>
		SoundData SoundLoadMp3(const std::wstring path);

		/// <summary>
		/// 音声データの解放
		/// </summary>
		void SoundUnload();
	};
}
