#include "Constants.h"
#include "RuntimeImp.h"
#include <chrono>
#include <device-layer/IDeviceLayer.h>
#include <fstream>
#include <hostUtils/logging/Logging.h>
#include <thread>

using namespace rt;

inline std::vector<std::byte> readFile(const std::string& path) {
  auto file = std::ifstream(path, std::ios_base::binary);
  if (!file.is_open()) {
    throw std::runtime_error("can't open file" + path);
  }
  auto iniF = file.tellg();
  file.seekg(0, std::ios::end);
  auto endF = file.tellg();
  auto size = endF - iniF;
  file.seekg(0, std::ios::beg);

  std::vector<std::byte> fileContent(static_cast<uint32_t>(size));
  file.read(reinterpret_cast<char*>(fileContent.data()), size);
  return fileContent;
}

inline auto getDefaultSysemuOptions() {
  constexpr uint64_t kSysEmuMaxCycles = std::numeric_limits<uint64_t>::max();
  constexpr uint64_t kSysEmuMinionShiresMask = 0x1FFFFFFFFu;

  emu::SysEmuOptions sysEmuOptions;
  sysEmuOptions.bootromTrampolineToBL2ElfPath = BOOTROM_TRAMPOLINE_TO_BL2_ELF;
  sysEmuOptions.spBL2ElfPath = BL2_ELF;
  sysEmuOptions.machineMinionElfPath = MACHINE_MINION_ELF;
  sysEmuOptions.masterMinionElfPath = MASTER_MINION_ELF;
  sysEmuOptions.workerMinionElfPath = WORKER_MINION_ELF;
  sysEmuOptions.executablePath = std::string(SYSEMU_INSTALL_DIR) + "sys_emu";
  sysEmuOptions.runDir = std::filesystem::current_path();
  sysEmuOptions.maxCycles = kSysEmuMaxCycles;
  sysEmuOptions.minionShiresMask = kSysEmuMinionShiresMask;
  sysEmuOptions.puUart0Path = sysEmuOptions.runDir + "/pu_uart0_tx.log";
  sysEmuOptions.puUart1Path = sysEmuOptions.runDir + "/pu_uart1_tx.log";
  sysEmuOptions.spUart0Path = sysEmuOptions.runDir + "/spio_uart0_tx.log";
  sysEmuOptions.spUart1Path = sysEmuOptions.runDir + "/spio_uart1_tx.log";
  sysEmuOptions.startGdb = false;
  return sysEmuOptions;
}

int main(int argc, char* argv[]) {
  using namespace std::chrono_literals;

#ifdef ENABLE_LINUX_DRIVER
  std::shared_ptr<dev::IDeviceLayer> deviceLayer = dev::IDeviceLayer::createPcieDeviceLayer();
#else
  std::shared_ptr<dev::IDeviceLayer> deviceLayer = dev::IDeviceLayer::createSysEmuDeviceLayer(getDefaultSysemuOptions());
#endif
  auto runtime = rt::IRuntime::create(deviceLayer);
  auto devices = runtime->getDevices();
  auto st = runtime->createStream(devices[0]);
  auto fileContents = readFile(argv[1]);
  auto kernel = runtime->loadCode(st, fileContents.data(), fileContents.size());
  runtime->waitForStream(st);
  auto rimp = static_cast<rt::RuntimeImp*>(runtime.get());
  volatile bool done = false;
  rimp->setSentCommandCallback(devices[0], [&done](rt::Command const* cmd) { done = true; });
  std::array<std::byte, 4> junk;
  runtime->kernelLaunch(st, kernel.kernel_, junk.data(), junk.size(), 0x1FFFFFFFFUL);
  LOG(INFO) << "Waiting for command to be sent...";
  while (!done) {
    std::this_thread::sleep_for(1ms);
  }
  LOG(INFO) << "Kernel launched.";
}
