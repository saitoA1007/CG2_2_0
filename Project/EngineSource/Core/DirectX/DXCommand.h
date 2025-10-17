#pragma once
#include <d3d12.h>
#include <wrl.h>

namespace GameEngine {

	class DXCommand {
	public:

		DXCommand() = default;
		~DXCommand() = default;

		void Initialize(ID3D12Device* device);

		void Close();

		void Execute();

		void Reset();

		ID3D12CommandQueue* GetQueue() const { return commandQueue_.Get(); }

		ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	};
}

