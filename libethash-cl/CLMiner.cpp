/// OpenCL miner implementation.
///
/// @file
/// @copyright GNU General Public License

#include "CLMiner.h"
#include <libethash/internal.h>
#include "CLMiner_kernel_stable.h"
#include "CLMiner_kernel_experimental.h"
#include "CLMiner_kernel_ethash_new.h"
#include "CLMiner_kernel_ethash_genoil.h"
#include "CLMiner_kernel_ethash_old.h"
#include "CLMiner_kernel_basic.h"


using namespace dev;
using namespace eth;

namespace dev
{
namespace eth
{

unsigned CLMiner::s_workgroupSize = CLMiner::c_defaultLocalWorkSize;
unsigned CLMiner::s_initialGlobalWorkSize = CLMiner::c_defaultGlobalWorkSizeMultiplier * CLMiner::c_defaultLocalWorkSize;
unsigned CLMiner::s_threadsPerHash = 8;
CLKernelName CLMiner::s_clKernelName = CLMiner::c_defaultKernelName;

constexpr size_t c_maxSearchResults = 1;

struct CLChannel: public LogChannel
{
	static const char* name() { return EthOrange " cl"; }
	static const int verbosity = 2;
	static const bool debug = false;
};
struct CLSwitchChannel: public LogChannel
{
	static const char* name() { return EthOrange " cl"; }
	static const int verbosity = 6;
	static const bool debug = false;
};
#define cllog clog(CLChannel)
#define clswitchlog clog(CLSwitchChannel)
#define ETHCL_LOG(_contents) cllog << _contents

/**
 * Returns the name of a numerical cl_int error
 * Takes constants from CL/cl.h and returns them in a readable format
 */
static const char *strClError(cl_int err) {

	switch (err) {
	case CL_SUCCESS:
		return "CL_SUCCESS";
	case CL_DEVICE_NOT_FOUND:
		return "CL_DEVICE_NOT_FOUND";
	case CL_DEVICE_NOT_AVAILABLE:
		return "CL_DEVICE_NOT_AVAILABLE";
	case CL_COMPILER_NOT_AVAILABLE:
		return "CL_COMPILER_NOT_AVAILABLE";
	case CL_MEM_OBJECT_ALLOCATION_FAILURE:
		return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case CL_OUT_OF_RESOURCES:
		return "CL_OUT_OF_RESOURCES";
	case CL_OUT_OF_HOST_MEMORY:
		return "CL_OUT_OF_HOST_MEMORY";
	case CL_PROFILING_INFO_NOT_AVAILABLE:
		return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case CL_MEM_COPY_OVERLAP:
		return "CL_MEM_COPY_OVERLAP";
	case CL_IMAGE_FORMAT_MISMATCH:
		return "CL_IMAGE_FORMAT_MISMATCH";
	case CL_IMAGE_FORMAT_NOT_SUPPORTED:
		return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case CL_BUILD_PROGRAM_FAILURE:
		return "CL_BUILD_PROGRAM_FAILURE";
	case CL_MAP_FAILURE:
		return "CL_MAP_FAILURE";
	case CL_MISALIGNED_SUB_BUFFER_OFFSET:
		return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
		return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";

#ifdef CL_VERSION_1_2
	case CL_COMPILE_PROGRAM_FAILURE:
		return "CL_COMPILE_PROGRAM_FAILURE";
	case CL_LINKER_NOT_AVAILABLE:
		return "CL_LINKER_NOT_AVAILABLE";
	case CL_LINK_PROGRAM_FAILURE:
		return "CL_LINK_PROGRAM_FAILURE";
	case CL_DEVICE_PARTITION_FAILED:
		return "CL_DEVICE_PARTITION_FAILED";
	case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
		return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
#endif // CL_VERSION_1_2

	case CL_INVALID_VALUE:
		return "CL_INVALID_VALUE";
	case CL_INVALID_DEVICE_TYPE:
		return "CL_INVALID_DEVICE_TYPE";
	case CL_INVALID_PLATFORM:
		return "CL_INVALID_PLATFORM";
	case CL_INVALID_DEVICE:
		return "CL_INVALID_DEVICE";
	case CL_INVALID_CONTEXT:
		return "CL_INVALID_CONTEXT";
	case CL_INVALID_QUEUE_PROPERTIES:
		return "CL_INVALID_QUEUE_PROPERTIES";
	case CL_INVALID_COMMAND_QUEUE:
		return "CL_INVALID_COMMAND_QUEUE";
	case CL_INVALID_HOST_PTR:
		return "CL_INVALID_HOST_PTR";
	case CL_INVALID_MEM_OBJECT:
		return "CL_INVALID_MEM_OBJECT";
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
		return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case CL_INVALID_IMAGE_SIZE:
		return "CL_INVALID_IMAGE_SIZE";
	case CL_INVALID_SAMPLER:
		return "CL_INVALID_SAMPLER";
	case CL_INVALID_BINARY:
		return "CL_INVALID_BINARY";
	case CL_INVALID_BUILD_OPTIONS:
		return "CL_INVALID_BUILD_OPTIONS";
	case CL_INVALID_PROGRAM:
		return "CL_INVALID_PROGRAM";
	case CL_INVALID_PROGRAM_EXECUTABLE:
		return "CL_INVALID_PROGRAM_EXECUTABLE";
	case CL_INVALID_KERNEL_NAME:
		return "CL_INVALID_KERNEL_NAME";
	case CL_INVALID_KERNEL_DEFINITION:
		return "CL_INVALID_KERNEL_DEFINITION";
	case CL_INVALID_KERNEL:
		return "CL_INVALID_KERNEL";
	case CL_INVALID_ARG_INDEX:
		return "CL_INVALID_ARG_INDEX";
	case CL_INVALID_ARG_VALUE:
		return "CL_INVALID_ARG_VALUE";
	case CL_INVALID_ARG_SIZE:
		return "CL_INVALID_ARG_SIZE";
	case CL_INVALID_KERNEL_ARGS:
		return "CL_INVALID_KERNEL_ARGS";
	case CL_INVALID_WORK_DIMENSION:
		return "CL_INVALID_WORK_DIMENSION";
	case CL_INVALID_WORK_GROUP_SIZE:
		return "CL_INVALID_WORK_GROUP_SIZE";
	case CL_INVALID_WORK_ITEM_SIZE:
		return "CL_INVALID_WORK_ITEM_SIZE";
	case CL_INVALID_GLOBAL_OFFSET:
		return "CL_INVALID_GLOBAL_OFFSET";
	case CL_INVALID_EVENT_WAIT_LIST:
		return "CL_INVALID_EVENT_WAIT_LIST";
	case CL_INVALID_EVENT:
		return "CL_INVALID_EVENT";
	case CL_INVALID_OPERATION:
		return "CL_INVALID_OPERATION";
	case CL_INVALID_GL_OBJECT:
		return "CL_INVALID_GL_OBJECT";
	case CL_INVALID_BUFFER_SIZE:
		return "CL_INVALID_BUFFER_SIZE";
	case CL_INVALID_MIP_LEVEL:
		return "CL_INVALID_MIP_LEVEL";
	case CL_INVALID_GLOBAL_WORK_SIZE:
		return "CL_INVALID_GLOBAL_WORK_SIZE";
	case CL_INVALID_PROPERTY:
		return "CL_INVALID_PROPERTY";

#ifdef CL_VERSION_1_2
	case CL_INVALID_IMAGE_DESCRIPTOR:
		return "CL_INVALID_IMAGE_DESCRIPTOR";
	case CL_INVALID_COMPILER_OPTIONS:
		return "CL_INVALID_COMPILER_OPTIONS";
	case CL_INVALID_LINKER_OPTIONS:
		return "CL_INVALID_LINKER_OPTIONS";
	case CL_INVALID_DEVICE_PARTITION_COUNT:
		return "CL_INVALID_DEVICE_PARTITION_COUNT";
#endif // CL_VERSION_1_2

#ifdef CL_VERSION_2_0
	case CL_INVALID_PIPE_SIZE:
		return "CL_INVALID_PIPE_SIZE";
	case CL_INVALID_DEVICE_QUEUE:
		return "CL_INVALID_DEVICE_QUEUE";
#endif // CL_VERSION_2_0

#ifdef CL_VERSION_2_2
	case CL_INVALID_SPEC_ID:
		return "CL_INVALID_SPEC_ID";
	case CL_MAX_SIZE_RESTRICTION_EXCEEDED:
		return "CL_MAX_SIZE_RESTRICTION_EXCEEDED";
#endif // CL_VERSION_2_2
	}

	return "Unknown CL error encountered";
}

/**
 * Prints cl::Errors in a uniform way
 * @param msg text prepending the error message
 * @param clerr cl:Error object
 *
 * Prints errors in the format:
 *      msg: what(), string err() (numeric err())
 */
static std::string ethCLErrorHelper(const char *msg, cl::Error const &clerr) {
	std::ostringstream osstream;
	osstream << msg << ": " << clerr.what() << ": " << strClError(clerr.err())
	         << " (" << clerr.err() << ")";
	return osstream.str();
}

namespace
{

void addDefinition(string& _source, char const* _id, unsigned _value)
{
	char buf[256];
	sprintf(buf, "#define %s %uu\n", _id, _value);
	_source.insert(_source.begin(), buf, buf + strlen(buf));
}

std::vector<cl::Platform> getPlatforms()
{
	vector<cl::Platform> platforms;
	try
	{
		cl::Platform::get(&platforms);
	}
	catch(cl::Error const& err)
	{
#if defined(CL_PLATFORM_NOT_FOUND_KHR)
		if (err.err() == CL_PLATFORM_NOT_FOUND_KHR)
			cwarn << "No OpenCL platforms found";
		else
#endif
			throw err;
	}
	return platforms;
}

std::vector<cl::Device> getDevices(std::vector<cl::Platform> const& _platforms, unsigned _platformId)
{
	vector<cl::Device> devices;
	size_t platform_num = min<size_t>(_platformId, _platforms.size() - 1);
	try
	{
		_platforms[platform_num].getDevices(
			CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR,
			&devices
		);
	}
	catch (cl::Error const& err)
	{
		// if simply no devices found return empty vector
		if (err.err() != CL_DEVICE_NOT_FOUND)
			throw err;
	}
	return devices;
}

}

}
}

unsigned CLMiner::s_platformId = 0;
unsigned CLMiner::s_numInstances = 0;
vector<int> CLMiner::s_devices(MAX_MINERS, -1);

CLMiner::CLMiner(FarmFace& _farm, unsigned _index):
	Miner("cl-", _farm, _index)
{}

CLMiner::~CLMiner()
{
	stopWorking();
	kick_miner();
}

void CLMiner::workLoop()
{
	// Memory for zero-ing buffers. Cannot be static because crashes on macOS.
	int32_t const c_zero = 0;
	int32_t const c_unread = -1;
	int32_t const c_invalid = -2;

	uint64_t startNonce = 0;
	
	float meanTime = 0;
	int count = 0;
	int benchmarkTime = 0;
	bool testEnds = false;


	// The work package currently processed by GPU.
	WorkPackage current;
	current.header = h256{1u};
	current.seed = h256{1u};
	WorkPackage w = work();
	bool first = true;
	try {
		while (true)
		{
			int32_t results[c_maxSearchResults + 1];
			if (!first) {
			results[0] = c_unread;
			// Read results.
			// TODO: could use pinned host pointer instead.
		//	cllog << "try to read results";
			m_queue.enqueueReadBuffer(m_searchBuffer, CL_FALSE, 0, sizeof(results), &results);
			}
		//	cllog << "waiting for processing or new header";
			while ((results[0] == c_unread || first )) {
				first = false;
				w = work();
				if (current.header != w.header)
				{	
					// New work received. Update GPU data.
					while (!w)
					{
						cllog << "No work. Pause for 3 s.";
						std::this_thread::sleep_for(std::chrono::seconds(3));
						w = work();
					}

					//cllog << "New work: header" << w.header << "target" << w.boundary.hex();

					if (current.seed != w.seed)
					{
						if (s_dagLoadMode == DAG_LOAD_MODE_SEQUENTIAL)
						{
							while (s_dagLoadIndex < index)
								this_thread::sleep_for(chrono::seconds(1));
							++s_dagLoadIndex;
						}
						cllog << "start init";
						init(w.seed);
					}

					// Upper 64 bits of the boundary.
					const uint64_t target = (uint64_t)(u64)((u256)w.boundary >> 192);
					assert(target > 0);
			//		cllog<<"switch header";

					// Update header constant buffer.
					
					m_invalidatingQueue.enqueueWriteBuffer(m_searchBuffer, CL_TRUE, 0, sizeof(c_invalid), &c_invalid);
					
					m_queue.enqueueWriteBuffer(m_header, CL_FALSE, 0, w.header.size, w.header.data());
					m_queue.enqueueWriteBuffer(m_searchBuffer, CL_FALSE, 0, sizeof(c_zero), &c_zero);
			//		cllog<<"search buffer invalidated";
					m_searchKernel.setArg(0, m_searchBuffer);  // Supply output buffer to kernel.
					m_searchKernel.setArg(4, target);
					
					current = w;        // kernel now processing newest work
					// FIXME: This logic should be move out of here.
					if (w.exSizeBits >= 0)
					{
						// This can support up to 2^c_log2MaxMiners devices.
						startNonce = w.startNonce | ((uint64_t)index << (64 - LOG2_MAX_MINERS - w.exSizeBits));
					}
					else
						startNonce = get_start_nonce();

					clswitchlog << "Switch time"
						<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - workSwitchStart).count()
						<< "ms.";
				
				}
			}
			
		//	cllog << "send invalidation of search buffer";
			uint64_t nonce = 0;
			if (results[0] > 0)
			{
\
				// Ignore results except the first one.
				nonce = current.startNonce + results[1];
				// Reset search buffer if any solution found.
				m_queue.enqueueWriteBuffer(m_searchBuffer, CL_FALSE, 0, sizeof(c_zero), &c_zero);

			}
			// Run the kernel.
			m_searchKernel.setArg(3, startNonce);

		//	cllog << "try to send NDRangeKernel";
		//	printMillis(388);
			m_queue.enqueueNDRangeKernel(m_searchKernel, cl::NullRange, m_globalWorkSize, m_workgroupSize);
			//cllog << "send NDRangeKernel";
			if (nonce != 0) {
				Result r = EthashAux::eval(current.seed, current.header, nonce);
                if (r.value < current.boundary) {
                    farm.submitProof(Solution{nonce, r.mixHash, current, current.header != w.header});
                }
				else {
					farm.failedSolution();
					cwarn << "FAILURE: GPU gave incorrect result!";
				}
			}
			
			if (checkTime()>599900){
				unsigned long hashResults[2];
				m_invalidatingQueue.enqueueReadBuffer(m_hashCountBuffer, CL_TRUE, 0, sizeof(hashResults), &hashResults);
				cllog<<"Hash count :"<<hashResults[0]<<";Invalid hash count :"<<hashResults[1]<<";Time = "<<checkTime();
			}

            
			current.startNonce = startNonce;
			// Increase start nonce for following kernel execution.
			startNonce += m_globalWorkSize;

			// Report hash count
			addHashCount(m_globalWorkSize);
			// Check if we should stop.
			if (shouldStop())
			{
				unsigned long hashResults[2];
				m_invalidatingQueue.enqueueReadBuffer(m_hashCountBuffer, CL_TRUE, 0, sizeof(hashResults), &hashResults);
				cllog<<"Hash count :"<<hashResults[0]<<";Invalid hash count :"<<hashResults[1]<<";Time = "<<checkTime();
				// Make sure the last buffer write has finished --
				// it reads local variable.
				m_queue.finish();
				break;
			}
		}
	}
	catch (cl::Error const& _e)
	{
		cwarn << ethCLErrorHelper("OpenCL Error", _e);
	}
}

void CLMiner::kick_miner() {}

unsigned CLMiner::getNumDevices()
{
	vector<cl::Platform> platforms = getPlatforms();
	if (platforms.empty())
		return 0;

	vector<cl::Device> devices = getDevices(platforms, s_platformId);
	if (devices.empty())
	{
		cwarn << "No OpenCL devices found.";
		return 0;
	}
	return devices.size();
}

void CLMiner::listDevices()
{
	string outString ="\nListing OpenCL devices.\nFORMAT: [platformID] [deviceID] deviceName\n";
	unsigned int i = 0;

	vector<cl::Platform> platforms = getPlatforms();
	if (platforms.empty())
		return;
	for (unsigned j = 0; j < platforms.size(); ++j)
	{
		i = 0;
		vector<cl::Device> devices = getDevices(platforms, j);
		for (auto const& device: devices)
		{
			outString += "[" + to_string(j) + "] [" + to_string(i) + "] " + device.getInfo<CL_DEVICE_NAME>() + "\n";
			outString += "\tCL_DEVICE_TYPE: ";
			switch (device.getInfo<CL_DEVICE_TYPE>())
			{
			case CL_DEVICE_TYPE_CPU:
				outString += "CPU\n";
				break;
			case CL_DEVICE_TYPE_GPU:
				outString += "GPU\n";
				break;
			case CL_DEVICE_TYPE_ACCELERATOR:
				outString += "ACCELERATOR\n";
				break;
			default:
				outString += "DEFAULT\n";
				break;
			}
			outString += "\tCL_DEVICE_GLOBAL_MEM_SIZE: " + to_string(device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>()) + "\n";
			outString += "\tCL_DEVICE_MAX_MEM_ALLOC_SIZE: " + to_string(device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>()) + "\n";
			outString += "\tCL_DEVICE_MAX_WORK_GROUP_SIZE: " + to_string(device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>()) + "\n";
			++i;
		}
	}
	std::cout << outString;
}

bool CLMiner::configureGPU(
	unsigned _localWorkSize,
	unsigned _globalWorkSizeMultiplier,
	unsigned _platformId,
	uint64_t _currentBlock,
	unsigned _dagLoadMode,
	unsigned _dagCreateDevice
)
{
	s_dagLoadMode = _dagLoadMode;
	s_dagCreateDevice = _dagCreateDevice;

	s_platformId = _platformId;

	_localWorkSize = ((_localWorkSize + 7) / 8) * 8;
	s_workgroupSize = _localWorkSize;
	s_initialGlobalWorkSize = _globalWorkSizeMultiplier * _localWorkSize;

	uint64_t dagSize = ethash_get_datasize(_currentBlock);

	vector<cl::Platform> platforms = getPlatforms();
	if (platforms.empty())
		return false;
	if (_platformId >= platforms.size())
		return false;

	vector<cl::Device> devices = getDevices(platforms, _platformId);
	for (auto const& device: devices)
	{
		cl_ulong deviceData = 0;
		device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &deviceData);
		printf("MAX_WORKGROUP_SIZE = %lu\n", deviceData);
		device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &deviceData);
		printf("CL_DEVICE_MAX_COMPUTE_UNITS = %lu\n", deviceData);
		cl_ulong result = 0;
		device.getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &result);
		if (result >= dagSize)
		{
			cnote <<
				"Found suitable OpenCL device [" << device.getInfo<CL_DEVICE_NAME>()
												 << "] with " << result << " bytes of GPU memory";
			return true;
		}

		cnote <<
			"OpenCL device " << device.getInfo<CL_DEVICE_NAME>()
							 << " has insufficient GPU memory." << result <<
							 " bytes of memory found < " << dagSize << " bytes of memory required";
	}

	cout << "No GPU device with sufficient memory was found. Can't GPU mine. Remove the -G argument" << endl;
	return false;
}

bool CLMiner::init(const h256& seed)
{
	EthashAux::LightType light = EthashAux::light(seed);

	// get all platforms
	try
	{
		vector<cl::Platform> platforms = getPlatforms();
		if (platforms.empty())
			return false;

		// use selected platform
		unsigned platformIdx = min<unsigned>(s_platformId, platforms.size() - 1);

		string platformName = platforms[platformIdx].getInfo<CL_PLATFORM_NAME>();
		ETHCL_LOG("Platform: " << platformName);

		int platformId = OPENCL_PLATFORM_UNKNOWN;
		{
			// this mutex prevents race conditions when calling the adl wrapper since it is apparently not thread safe
			static std::mutex mtx;
			std::lock_guard<std::mutex> lock(mtx);

			if (platformName == "NVIDIA CUDA")
			{
				platformId = OPENCL_PLATFORM_NVIDIA;
				m_hwmoninfo.deviceType = HwMonitorInfoType::NVIDIA;
			}
			else if (platformName == "AMD Accelerated Parallel Processing")
			{
				platformId = OPENCL_PLATFORM_AMD;
				m_hwmoninfo.deviceType = HwMonitorInfoType::AMD;
			}
			else if (platformName == "Clover")
			{
				platformId = OPENCL_PLATFORM_CLOVER;
			}
		}

		// get GPU device of the default platform
		vector<cl::Device> devices = getDevices(platforms, platformIdx);
		if (devices.empty())
		{
			ETHCL_LOG("No OpenCL devices found.");
			return false;
		}

		// use selected device
		unsigned deviceId = s_devices[index] > -1 ? s_devices[index] : index;
		m_hwmoninfo.deviceIndex = deviceId;
		cl::Device& device = devices[min<unsigned>(deviceId, devices.size() - 1)];
		string device_version = device.getInfo<CL_DEVICE_VERSION>();
		ETHCL_LOG("Device:   " << device.getInfo<CL_DEVICE_NAME>() << " / " << device_version);

		string clVer = device_version.substr(7, 3);
		if (clVer == "1.0" || clVer == "1.1")
		{
			if (platformId == OPENCL_PLATFORM_CLOVER)
			{
				ETHCL_LOG("OpenCL " << clVer << " not supported, but platform Clover might work nevertheless. USE AT OWN RISK!");
			}
			else
			{
				ETHCL_LOG("OpenCL " << clVer << " not supported - minimum required version is 1.2");
				return false;
			}
		}

		char options[256];
		int computeCapability = 0;
		if (platformId == OPENCL_PLATFORM_NVIDIA) {
			cl_uint computeCapabilityMajor;
			cl_uint computeCapabilityMinor;
			clGetDeviceInfo(device(), CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV, sizeof(cl_uint), &computeCapabilityMajor, NULL);
			clGetDeviceInfo(device(), CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV, sizeof(cl_uint), &computeCapabilityMinor, NULL);

			computeCapability = computeCapabilityMajor * 10 + computeCapabilityMinor;
			int maxregs = computeCapability >= 35 ? 72 : 63;
			sprintf(options, "-cl-nv-maxrregcount=%d", maxregs);
		}
		else {
			sprintf(options, "%s", "");
		}
		// create context
		m_context = cl::Context(vector<cl::Device>(&device, &device + 1));
		m_queue = cl::CommandQueue(m_context, device, CL_QUEUE_PROFILING_ENABLE);
		m_invalidatingQueue = cl::CommandQueue(m_context, device, CL_QUEUE_PROFILING_ENABLE);
		// make sure that global work size is evenly divisible by the local workgroup size
		m_workgroupSize = s_workgroupSize;
		m_globalWorkSize = s_initialGlobalWorkSize;
		if (m_globalWorkSize % m_workgroupSize != 0)
			m_globalWorkSize = ((m_globalWorkSize / m_workgroupSize) + 1) * m_workgroupSize;

		uint64_t dagSize = ethash_get_datasize(light->light->block_number);
		uint32_t dagSize128 = (unsigned)(dagSize / ETHASH_MIX_BYTES);
		uint32_t lightSize64 = (unsigned)(light->data().size() / sizeof(node));

		// patch source code
		// note: The kernels here are simply compiled version of the respective .cl kernels
		// into a byte array by bin2h.cmake. There is no need to load the file by hand in runtime
		// See libethash-cl/CMakeLists.txt: add_custom_command()
		// TODO: Just use C++ raw string literal.
		string code;
		switch (s_clKernelName) {
			case CLKernelName::EthashNew:
				cllog << "OpenCL kernel: EthashNew kernel";
				code = string(CLMiner_kernel_ethash_new, CLMiner_kernel_ethash_new + sizeof(CLMiner_kernel_ethash_new));
				break;
				
			case CLKernelName::EthashGenoil:
				cllog << "OpenCL kernel: EthashGenoil kernel";
				code = string(CLMiner_kernel_ethash_genoil, CLMiner_kernel_ethash_genoil + sizeof(CLMiner_kernel_ethash_genoil));
				break;
				
			case CLKernelName::EthashOld:
				cllog << "OpenCL kernel: EthashOld kernel";
				code = string(CLMiner_kernel_ethash_old, CLMiner_kernel_ethash_old + sizeof(CLMiner_kernel_ethash_old));
				break;
				
			case CLKernelName::Experimental:
				cllog << "OpenCL kernel: Experimental kernel";
				code = string(CLMiner_kernel_experimental, CLMiner_kernel_experimental + sizeof(CLMiner_kernel_experimental));
				break;
			case CLKernelName::Basic:
				cllog << "OpenCL kernel: Basic kernel";
				code = string(CLMiner_kernel_basic, CLMiner_kernel_basic + sizeof(CLMiner_kernel_basic));
				break;
			default:
				//if(s_clKernelName == CLKernelName::Stable)
				cllog << "OpenCL kernel: Stable kernel";

				//CLMiner_kernel_stable.cl will do a #undef THREADS_PER_HASH
				if(s_threadsPerHash != 8) {
					cwarn << "The current stable OpenCL kernel only supports exactly 8 threads. Thread parameter will be ignored.";
				}
				code = string(CLMiner_kernel_stable, CLMiner_kernel_stable + sizeof(CLMiner_kernel_stable));
				break;
		}
		
		
		addDefinition(code, "GROUP_SIZE", m_workgroupSize);
		addDefinition(code, "DAG_SIZE", dagSize128);
		addDefinition(code, "LIGHT_SIZE", lightSize64);
		addDefinition(code, "ACCESSES", ETHASH_ACCESSES);
		addDefinition(code, "MAX_OUTPUTS", c_maxSearchResults);
		addDefinition(code, "PLATFORM", platformId);
		addDefinition(code, "COMPUTE", computeCapability);
		addDefinition(code, "THREADS_PER_HASH", s_threadsPerHash);

		// create miner OpenCL program
		cl::Program::Sources sources{{code.data(), code.size()}};
		cl::Program program(m_context, sources);
		try
		{
			program.build({device}, options);
			cllog << "Build info:" << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
		}
		catch (cl::Error const&)
		{
			cwarn << "Build info:" << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
			return false;
		}

		//check whether the current dag fits in memory everytime we recreate the DAG
		cl_ulong result = 0;
		device.getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &result);
		if (result < dagSize)
		{
			cnote <<
			"OpenCL device " << device.getInfo<CL_DEVICE_NAME>()
							 << " has insufficient GPU memory." << result <<
							 " bytes of memory found < " << dagSize << " bytes of memory required";	
			return false;
		}

		// create buffer for dag
		try
		{
			cllog << "Creating light cache buffer, size" << light->data().size();
			m_light = cl::Buffer(m_context, CL_MEM_READ_ONLY, light->data().size());
			cllog << "Creating DAG buffer, size" << dagSize;
			m_dag = cl::Buffer(m_context, CL_MEM_READ_ONLY, dagSize);
			cllog << "Loading kernels";
			m_searchKernel = cl::Kernel(program, "ethash_search");
			m_dagKernel = cl::Kernel(program, "ethash_calculate_dag_item");
			cllog << "Writing light cache buffer";
			m_queue.enqueueWriteBuffer(m_light, CL_TRUE, 0, light->data().size(), light->data().data());
			
		}
		catch (cl::Error const& err)
		{
			cwarn << ethCLErrorHelper("Creating DAG buffer failed", err);
			return false;
		}
		// create buffer for header
		ETHCL_LOG("Creating buffer for header.");
		m_header = cl::Buffer(m_context, CL_MEM_READ_ONLY, 32);

		m_searchKernel.setArg(1, m_header);
		m_searchKernel.setArg(2, m_dag);
		m_searchKernel.setArg(5, ~0u);  // Pass this to stop the compiler unrolling the loops.

		// create mining buffers
		ETHCL_LOG("Creating mining buffer");
		m_searchBuffer = cl::Buffer(m_context, CL_MEM_WRITE_ONLY, (c_maxSearchResults + 1) * sizeof(int32_t));
		m_hashCountBuffer = cl::Buffer(m_context, CL_MEM_WRITE_ONLY, 2 * sizeof(unsigned long));
		
		unsigned long const c_zero = 0; 
		m_invalidatingQueue.enqueueWriteBuffer(m_hashCountBuffer, CL_TRUE, 0, sizeof(c_zero), &c_zero);
		m_searchKernel.setArg(6,m_hashCountBuffer);

		uint32_t const work = (uint32_t)(dagSize / sizeof(node));
		uint32_t fullRuns = work / m_globalWorkSize;
		uint32_t const restWork = work % m_globalWorkSize;
		if (restWork > 0) fullRuns++;

		m_dagKernel.setArg(1, m_light);
		m_dagKernel.setArg(2, m_dag);
		m_dagKernel.setArg(3, ~0u);

		auto startDAG = std::chrono::steady_clock::now();
		for (uint32_t i = 0; i < fullRuns; i++)
		{
			m_dagKernel.setArg(0, i * m_globalWorkSize);
			m_queue.enqueueNDRangeKernel(m_dagKernel, cl::NullRange, m_globalWorkSize, m_workgroupSize);
			m_queue.finish();
		}
		auto endDAG = std::chrono::steady_clock::now();

		auto dagTime = std::chrono::duration_cast<std::chrono::milliseconds>(endDAG-startDAG);
		float gb = (float)dagSize / (1024 * 1024 * 1024);
		cnote << gb << " GB of DAG data generated in" << dagTime.count() << "ms.";
		initCounter();
	}
	catch (cl::Error const& err)
	{
		cwarn << ethCLErrorHelper("OpenCL init failed", err);
		return false;
	}
	return true;
}
