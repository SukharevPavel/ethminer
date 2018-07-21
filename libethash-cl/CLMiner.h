/// OpenCL miner implementation.
///
/// @file
/// @copyright GNU General Public License

#pragma once

#include <libdevcore/Worker.h>
#include <libethcore/EthashAux.h>
#include <libethcore/Miner.h>
#include <fstream>
#include <chrono>

#pragma GCC diagnostic push
#if __GNUC__ >= 6
    #pragma GCC diagnostic ignored "-Wignored-attributes"
#endif
#pragma GCC diagnostic ignored "-Wmissing-braces"
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS true
#define CL_HPP_ENABLE_EXCEPTIONS true
#define CL_HPP_CL_1_2_DEFAULT_BUILD true
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#include "CL/cl2.hpp"
#pragma GCC diagnostic pop

// macOS OpenCL fix:
#ifndef CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV
#define CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV       0x4000
#endif

#ifndef CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV
#define CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV       0x4001
#endif

#define OPENCL_PLATFORM_UNKNOWN 0
#define OPENCL_PLATFORM_NVIDIA  1
#define OPENCL_PLATFORM_AMD     2
#define OPENCL_PLATFORM_CLOVER  3

using namespace std::chrono;

namespace dev
{
namespace eth
{

enum CLKernelName {
	Stable,
	Experimental,
    Binary
};

class CLMiner: public Miner
{
public:
	/* -- default values -- */
	/// Default value of the local work size. Also known as workgroup size.
	static const unsigned c_defaultLocalWorkSize = 128;
	/// Default value of the global work size as a multiplier of the local work size
	static const unsigned c_defaultGlobalWorkSizeMultiplier = 8192;

	/// Default value of the kernel is the original one
	static const CLKernelName c_defaultKernelName = CLKernelName::Stable;

	CLMiner(FarmFace& _farm, unsigned _index);
	~CLMiner() override;

	static unsigned instances() { return s_numInstances > 0 ? s_numInstances : 1; }
	static unsigned getNumDevices();
	static void listDevices();
    static bool configureGPU(unsigned _localWorkSize, int _globalWorkSizeMultiplier,
        unsigned _platformId, int epoch, unsigned _dagLoadMode, unsigned _dagCreateDevice,
        bool _noeval, bool _exit);
    static void setNumInstances(unsigned _instances) { s_numInstances = std::min<unsigned>(_instances, getNumDevices()); }
	static void setThreadsPerHash(unsigned _threadsPerHash){s_threadsPerHash = _threadsPerHash; }
	static void setDevices(const vector<unsigned>& _devices, unsigned _selectedDeviceCount)
	{
		for (unsigned i = 0; i < _selectedDeviceCount; i++)
		{
			s_devices[i] = _devices[i];
		}
	}
	static void setCLKernel(unsigned _clKernel) {
		switch (_clKernel) {
			default: ;
			case 0: s_clKernelName = CLKernelName::Stable; break;
			case 1: s_clKernelName = CLKernelName::Experimental; break;
			case 2: s_clKernelName = CLKernelName::Binary; break;
		}
	}
    static void setNumberIterations(unsigned _iterations) {s_kernelIterations = _iterations <= 1 ? 1 : _iterations;}
protected:
	void kick_miner() override;

private:
	void workLoop() override;

 milliseconds initMs;
    unsigned long hashCount;
    unsigned long lostHashCount;
	unsigned long changeBlockCount;
	unsigned long openclCycleCount = 0;
    bool wasInvalidHeader = false;
	int curTime;

    void initCounter(){
        initMs = duration_cast< milliseconds >(
                    system_clock::now().time_since_epoch());
        hashCount = 0;
        lostHashCount = 0;
		openclCycleCount = 0;
		changeBlockCount = 0;
    }

    int checkTime(){
        milliseconds curMs = duration_cast< milliseconds >(
                    system_clock::now().time_since_epoch());
		curTime = curMs.count() - initMs.count();
		return curTime;
    }
	
	int getTime(){
		return curTime;
}

	bool init(int epoch);

	cl::Context m_context;
	cl::CommandQueue m_queue;
	cl::Kernel m_searchKernel;
	cl::Kernel m_dagKernel;

	cl::Buffer m_dag;
	cl::Buffer m_light;
	cl::Buffer m_header;
	cl::Buffer m_searchBuffer;
	unsigned m_globalWorkSize = 0;
	unsigned m_workgroupSize = 0;

	static unsigned s_platformId;
	static unsigned s_numInstances;
	static unsigned s_threadsPerHash;
	static CLKernelName s_clKernelName;
    static unsigned s_kernelIterations;
	static vector<int> s_devices;

	/// The local work size for the search
	static unsigned s_workgroupSize;
	/// The initial global work size for the searches
	static unsigned s_initialGlobalWorkSize;
	static bool s_adjustWorkSize;

};

}
}
