#pragma once

namespace Crimson
{

	class D3D12Instance
	{
	public:
		static void Init(bool enable_debug_extension);
		static void Dispose();
		static D3D12Instance* Get();
	private:
		D3D12Instance(bool enable_debug_extension);
		~D3D12Instance();


		static D3D12Instance* p_Singleton;
	};
}