
#pragma once
#include <windows.h>
#include <winver.h>
#include <Poco/Mutex.h>
#include "Poco/SharedPtr.h"
#include "Poco/Util/WinService.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/RegularExpression.h"
#include "CRC32.h"
#include "../Logger.h"
class Control {
private:
    Poco::SharedPtr<Poco::Util::WinService> poco_agent;
    Poco::File plugins_directory = Poco::Path::current() + "Plugins";
    Poco::File agent_file = Poco::Path::current() + "PocoAgent.exe";
    Poco::File my_file = Poco::Path::current() + "ControlService.exe";
    Poco::Mutex mutex;
public:
    Control() {
        poco_agent=Poco::makeShared<Poco::Util::WinService>("PocoAgent");
        poco_agent->setStartup(Poco::Util::WinService::SVC_MANUAL_START);
        if(!poco_agent->isRegistered())
            poco_agent->registerService("\""+agent_file.path()+"\"");
    }

    ~Control(){
    }

    void startAgent() {
        if (poco_agent->isStopped())
            poco_agent->start();
    }

    bool getAgentStatus(){
        if(poco_agent->isRunning())
            return true;
        return false;
    }

    /*std::string getPluginsSize() {
        std::vector<Poco::File> all_plugins;
        std::string result;
        std::string temp;
        Poco::RegularExpression dll(R"([^\\|^\|^/]*[.]dll|so)");
        plugins_directory.list(all_plugins);
        for (auto & plugin : all_plugins)
            if(dll.extract(plugin.path(),temp,0))
                result += temp + " " + std::to_string(plugin.getSize()) + " Bytes\n";
        return result;
    }*/

    static long int getSize(const std::string& path) {
        Poco::File file(path);
        if(file.exists())
            return file.getSize();
        else
            return 0;
    }

    static unsigned int getControlSum(const std::string& path){
        Poco::File file(path);
        if(file.exists())
            return CRC32Coder::Crc32forFile(file.path());
        else
            return 0;
    }

    static std::string getVersion(const std::string& path) {
        std::string version = "error";
        DWORD verHandle = 0;
        UINT size = 0;
        LPBYTE lpBuffer = nullptr;
        std::wstring char_path = std::wstring(path.begin(), path.end());
        auto szVersionFile = const_cast<WCHAR *>(char_path.c_str());
        DWORD verSize = GetFileVersionInfoSize(szVersionFile, &verHandle);

        if (verSize != 0) {
            auto verData = new char[verSize];

            if (GetFileVersionInfo(szVersionFile, verHandle, verSize, verData)) {
                if (VerQueryValue(verData, TEXT("\\"), (VOID FAR *FAR *) &lpBuffer, &size)) {
                    if (size) {
                        auto *verInfo = (VS_FIXEDFILEINFO *) lpBuffer;
                        if (verInfo->dwSignature == 0xfeef04bd) {
                            // Doesn't matter if you are on 32 bit or 64 bit,
                            // DWORD is always 32 bits, so first two revision numbers
                            // come from dwFileVersionMS, last two come from dwFileVersionLS
                            version =
                                    "File Version:" + std::to_string((verInfo->dwFileVersionMS >> 16) & 0xffff) + "." +
                                    std::to_string((verInfo->dwFileVersionMS >> 0) & 0xffff) + "." +
                                    std::to_string((verInfo->dwFileVersionLS >> 16) & 0xffff) + "." +
                                    std::to_string((verInfo->dwFileVersionLS >> 0) & 0xffff) + "\n";
                        }
                    }
                }
            }
            delete[] verData;
        }
        return version;
    }
};