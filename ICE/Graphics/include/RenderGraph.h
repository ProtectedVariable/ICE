//
// Render Graph System for ICE Engine
// Enables flexible multi-pass rendering with automatic resource management
//

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>

#include "Framebuffer.h"
#include "GraphicsFactory.h"

namespace ICE {

// Forward declarations
class RenderGraphResource;
class RenderGraphPass;
class RenderGraph;

enum class ResourceType {
    Texture2D,
    TextureCube,
    Buffer,
    RenderTarget
};

struct ResourceDescriptor {
    ResourceType type;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t format = 0;  // GL format
    bool is_transient = true;  // Can be aliased/reused
    std::string debug_name;
};

// Represents a virtual resource in the render graph
class RenderGraphResource {
public:
    RenderGraphResource(const std::string& name, const ResourceDescriptor& desc)
        : m_name(name), m_descriptor(desc) {}
    
    const std::string& getName() const { return m_name; }
    const ResourceDescriptor& getDescriptor() const { return m_descriptor; }
    
    void setPhysicalResource(std::shared_ptr<void> resource) { 
        m_physical_resource = resource; 
    }
    
    std::shared_ptr<void> getPhysicalResource() const { 
        return m_physical_resource; 
    }
    
    template<typename T>
    std::shared_ptr<T> getPhysicalResourceAs() const {
        return std::static_pointer_cast<T>(m_physical_resource);
    }

private:
    std::string m_name;
    ResourceDescriptor m_descriptor;
    std::shared_ptr<void> m_physical_resource;
};

// Represents a single render pass in the graph
class RenderGraphPass {
public:
    using ExecuteCallback = std::function<void(const RenderGraphPass&)>;
    
    RenderGraphPass(const std::string& name) : m_name(name) {}
    
    // Resource declaration API
    void read(const std::string& resource_name) {
        m_reads.push_back(resource_name);
    }
    
    void write(const std::string& resource_name) {
        m_writes.push_back(resource_name);
    }
    
    void create(const std::string& resource_name, const ResourceDescriptor& desc) {
        m_creates[resource_name] = desc;
    }
    
    void setExecuteCallback(ExecuteCallback callback) {
        m_execute_callback = callback;
    }
    
    void execute() const {
        if (m_execute_callback) {
            m_execute_callback(*this);
        }
    }
    
    // Getters
    const std::string& getName() const { return m_name; }
    const std::vector<std::string>& getReads() const { return m_reads; }
    const std::vector<std::string>& getWrites() const { return m_writes; }
    const std::unordered_map<std::string, ResourceDescriptor>& getCreates() const {
        return m_creates;
    }
    
    // Resource access during execution
    template<typename T>
    std::shared_ptr<T> getResource(const std::string& name) const {
        auto it = m_resource_cache.find(name);
        if (it != m_resource_cache.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        return nullptr;
    }
    
    void cacheResource(const std::string& name, std::shared_ptr<void> resource) {
        m_resource_cache[name] = resource;
    }

private:
    std::string m_name;
    std::vector<std::string> m_reads;
    std::vector<std::string> m_writes;
    std::unordered_map<std::string, ResourceDescriptor> m_creates;
    ExecuteCallback m_execute_callback;
    mutable std::unordered_map<std::string, std::shared_ptr<void>> m_resource_cache;
};

// Main render graph class
class RenderGraph {
public:
    RenderGraph(const std::shared_ptr<GraphicsFactory>& factory) 
        : m_factory(factory) {}
    
    // Pass creation
    RenderGraphPass& addPass(const std::string& name) {
        auto pass = std::make_unique<RenderGraphPass>(name);
        auto& ref = *pass;
        m_passes.push_back(std::move(pass));
        return ref;
    }
    
    // Compile the graph (resolve dependencies, allocate resources)
    void compile() {
        buildDependencyGraph();
        topologicalSort();
        allocateResources();
        cullUnusedPasses();
    }
    
    // Execute all passes in dependency order
    void execute() {
        for (auto* pass : m_sorted_passes) {
            if (pass) {
                pass->execute();
            }
        }
    }
    
    // Reset graph for rebuilding
    void reset() {
        m_passes.clear();
        m_sorted_passes.clear();
        m_resources.clear();
        m_dependencies.clear();
    }
    
    // Get a resource by name
    std::shared_ptr<RenderGraphResource> getResource(const std::string& name) {
        auto it = m_resources.find(name);
        if (it != m_resources.end()) {
            return it->second;
        }
        return nullptr;
    }

private:
    void buildDependencyGraph() {
        m_dependencies.clear();
        
        // Build adjacency list: pass -> passes it depends on
        for (const auto& pass : m_passes) {
            for (const auto& read : pass->getReads()) {
                // Find which pass writes this resource
                for (const auto& other_pass : m_passes) {
                    if (other_pass.get() == pass.get()) continue;
                    
                    const auto& writes = other_pass->getWrites();
                    const auto& creates = other_pass->getCreates();
                    
                    bool writes_resource = std::find(writes.begin(), writes.end(), read) != writes.end();
                    bool creates_resource = creates.find(read) != creates.end();
                    
                    if (writes_resource || creates_resource) {
                        // pass depends on other_pass
                        m_dependencies[pass.get()].push_back(other_pass.get());
                    }
                }
            }
        }
    }
    
    void topologicalSort() {
        m_sorted_passes.clear();
        std::unordered_set<RenderGraphPass*> visited;
        std::unordered_set<RenderGraphPass*> temp_mark;
        
        for (const auto& pass : m_passes) {
            if (visited.find(pass.get()) == visited.end()) {
                topologicalSortVisit(pass.get(), visited, temp_mark);
            }
        }
        
        std::reverse(m_sorted_passes.begin(), m_sorted_passes.end());
    }
    
    void topologicalSortVisit(RenderGraphPass* pass,
                              std::unordered_set<RenderGraphPass*>& visited,
                              std::unordered_set<RenderGraphPass*>& temp_mark) {
        if (temp_mark.find(pass) != temp_mark.end()) {
            throw std::runtime_error("Render graph has cycles!");
        }
        
        if (visited.find(pass) != visited.end()) {
            return;
        }
        
        temp_mark.insert(pass);
        
        if (m_dependencies.find(pass) != m_dependencies.end()) {
            for (auto* dep : m_dependencies[pass]) {
                topologicalSortVisit(dep, visited, temp_mark);
            }
        }
        
        temp_mark.erase(pass);
        visited.insert(pass);
        m_sorted_passes.push_back(pass);
    }
    
    void allocateResources() {
        // Create physical resources for all declared resources
        for (const auto& pass : m_passes) {
            for (const auto& [name, desc] : pass->getCreates()) {
                if (m_resources.find(name) == m_resources.end()) {
                    auto resource = std::make_shared<RenderGraphResource>(name, desc);
                    
                    // Allocate physical resource based on type
                    if (desc.type == ResourceType::RenderTarget) {
                        FrameBufferFormat format{desc.width, desc.height, 1};
                        auto fb = m_factory->createFramebuffer(format);
                        resource->setPhysicalResource(fb);
                    }
                    // TODO: Handle other resource types (Texture2D, Buffer, etc.)
                    
                    m_resources[name] = resource;
                    pass->cacheResource(name, resource->getPhysicalResource());
                }
            }
        }
        
        // Cache resources for passes that read them
        for (const auto& pass : m_passes) {
            for (const auto& read : pass->getReads()) {
                if (m_resources.find(read) != m_resources.end()) {
                    pass->cacheResource(read, m_resources[read]->getPhysicalResource());
                }
            }
            for (const auto& write : pass->getWrites()) {
                if (m_resources.find(write) != m_resources.end()) {
                    pass->cacheResource(write, m_resources[write]->getPhysicalResource());
                }
            }
        }
    }
    
    void cullUnusedPasses() {
        // TODO: Implement pass culling based on which resources are actually used
        // For now, keep all passes
    }

private:
    std::shared_ptr<GraphicsFactory> m_factory;
    std::vector<std::unique_ptr<RenderGraphPass>> m_passes;
    std::vector<RenderGraphPass*> m_sorted_passes;
    std::unordered_map<std::string, std::shared_ptr<RenderGraphResource>> m_resources;
    std::unordered_map<RenderGraphPass*, std::vector<RenderGraphPass*>> m_dependencies;
};

}  // namespace ICE
