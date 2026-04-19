#pragma once
#include "scriptAssemblyData.h"
#include "scriptContext.h"
#include "asset/assetRef.h"
#include "Coral/HostInstance.hpp"

namespace ion
{
class ScriptManager
{
public:
    ScriptManager() = default;

    ScriptManager(const ScriptManager&) = delete;
    ScriptManager& operator=(const ScriptManager&) = delete;

    void init();

    ScriptContext& createContext(const std::string &name);
    void unloadContext(const std::string &name);
    ScriptContext* getContext(const std::string &name);
private:
    std::unordered_map<std::string, AssetRef<ScriptAssemblyData>> m_pendingAssemblies;

    Coral::HostInstance m_hostInstance;
    std::unordered_map<std::string, ScriptContext> m_scriptContexts;
     AssetRef<ScriptAssemblyData> m_engineAssemblyData;
};
}
