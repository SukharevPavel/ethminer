/// OpenCL miner implementation.
///
/// @file
/// @copyright GNU General Public License

#pragma once

#include <fstream>

#include <libdevcore/Worker.h>
#include <libethcore/EthashAux.h>
#include <libethcore/Miner.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
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
#define CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV 0x4000
#endif

#ifndef CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV
#define CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV 0x4001
#endif

using namespace std::chrono;

namespace dev
{
namespace eth
{
class CLMiner : public Miner
{
public:

    CLMiner(unsigned _index, CLSettings _settings, DeviceDescriptor& _device);
    ~CLMiner() override;

    static void enumDevices(std::map<string, DeviceDescriptor>& _DevicesCollection);

protected:
    bool initDevice() override;

    bool initEpoch_internal() override;

    void kick_miner() override;

private:

    void workLoop() override;

    vector<cl::Context> m_context;
    vector<cl::CommandQueue> m_queue;
    vector<cl::CommandQueue> m_abortqueue;
    cl::Kernel m_searchKernel;
    cl::Kernel m_dagKernel;
    cl::Device m_device;

    vector<cl::Buffer> m_dag;
    vector<cl::Buffer> m_light;
    vector<cl::Buffer> m_header;
    vector<cl::Buffer> m_searchBuffer;

    CLSettings m_settings;

    unsigned m_dagItems = 0;
    uint64_t m_lastNonce = 0;

    milliseconds initMs;
	milliseconds lastCycleTime;
	int curTime;
	unsigned long hashCount;
    unsigned long lostHashCount;
	unsigned long changeBlockCount;
	unsigned long openclCycleCount = 0;
	bool wasInvalidHeader = false;
	unsigned int meanCycleTime;
	unsigned int cycleCount;
	uint64_t kernelHashCount;
	vector<cl::Buffer> m_verifyBuffer;
	vector<cl::Buffer> m_verifyResultBuffer;

    void initCounter(){
        initMs = duration_cast< milliseconds >(
                    system_clock::now().time_since_epoch());
		hashCount = 0;
        lostHashCount = 0;
		openclCycleCount = 0;
		changeBlockCount = 0;
		meanCycleTime = 0;
		cycleCount = 0;
		kernelHashCount = 0;
		lastCycleTime = duration_cast< milliseconds >(steady_clock::duration::zero());
    }

    void verify(uint32_t *verified, uint shouldBeVerifiedCount, WorkPackage current);

    int checkTime(){
        milliseconds curMs = duration_cast< milliseconds >(
                    system_clock::now().time_since_epoch());
		curTime = curMs.count() - initMs.count();
		return curTime;
    }

    int checkCycleTime(){
    	if (lastCycleTime.count()) {
        milliseconds curMs = duration_cast< milliseconds >(
                    system_clock::now().time_since_epoch());
		curTime = curMs.count() - lastCycleTime.count();
		lastCycleTime = curMs;
		meanCycleTime = (meanCycleTime * cycleCount + curTime) / (cycleCount + 1);
		cycleCount++;
	} else {
			lastCycleTime =  duration_cast< milliseconds >(
                    system_clock::now().time_since_epoch());
		}
		return meanCycleTime;

    }

	int getTime(){
		return curTime;
	}

	milliseconds lastMs;

	void printMillis(int num){
		milliseconds ms = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch());
		printf("on line %d for %lu ms\n", num, ms - lastMs);
		lastMs = ms;
	}

};

}  // namespace eth
}  // namespace dev
