#include "ImGuiManager.h"

#include <externals/imgui/imgui.h>
#include <externals/imgui/imgui_impl_dx12.h>
#include <externals/imgui/imgui_impl_win32.h>

void ImGuiManager::Init(WinApp* winApp, DirectXCommon* dxCommon) {

	HRESULT hr;

	winApp_ = winApp;
	dxCommon_ = dxCommon;

	// ImGuiのコンテキストを生成
	ImGui::CreateContext();
	// ImGuiのスタイルを設定
	ImGui::StyleColorsDark();

	// Win32用の初期化
	ImGui_ImplWin32_Init(winApp_->GetHwnd());

	// デスクリプタヒープ設定
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	// デスクリプタヒープ生成
	hr = dxCommon_->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvHeap_));
	assert(SUCCEEDED(hr));

	// DX12用の初期化
	ImGui_ImplDX12_Init(
	    dxCommon_->GetDevice(), static_cast<int>(dxCommon_->GetBackBufferCount()), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, srvHeap_.Get(), srvHeap_->GetCPUDescriptorHandleForHeapStart(),
	    srvHeap_->GetGPUDescriptorHandleForHeapStart());

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void ImGuiManager::Finalize() {
	
	// 後始末
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// デスクリプタヒープを解放
	srvHeap_.Reset();
}

void ImGuiManager::Begin() {

	// ImGuiフレーム開始
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::End() {

	// 描画前準備
	ImGui::Render();
}

void ImGuiManager::Draw() {
	
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// デスクリプタヒープ
	ID3D12DescriptorHeap* ppHeaps[] = {srvHeap_.Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	// 描画コマンドを発行
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}