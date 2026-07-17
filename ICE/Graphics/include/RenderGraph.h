//
// Render Graph System for ICE Engine
// Enables flexible multi-pass rendering with automatic resource management
//

#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
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
class RenderGraphBuilder;
class PassContext;

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
    // Backend-agnostic pixel format (was a raw GL enum, which no code set and which no non-GL
    // backend could honour). Used when the graph allocates a Texture2D.
    TextureFormat format = TextureFormat::RGBA8;
    bool is_transient = true;  // may be pooled/reused across rebuilds; false = keep contents
    std::string debug_name;
};

// Maps a physical resource type to the graph resource type it is allocated as. This is what makes
// create<T>() type-directed: instantiating it for a type with no specialization here is a compile
// error, so an unsupported resource can never be declared. (TextureCube/Buffer specializations
// arrive with their allocation support in T5.)
template<typename T>
struct ResourceTypeOf;

template<>
struct ResourceTypeOf<Framebuffer> {
    static constexpr ResourceType value = ResourceType::RenderTarget;
};

template<>
struct ResourceTypeOf<GPUTexture> {
    static constexpr ResourceType value = ResourceType::Texture2D;
};

// A typed, opaque reference to an entry in the graph's resource table.
//
// Handles are the only way to name a resource in the typed pass API. You cannot mistype one (there
// is no string to get wrong), and using a resource as the wrong type is a compile error rather than
// the null physical pointer that getResource<T>("typo") hands back at draw time.
//
// Lifetime: a handle is an index into the *current* resource table, so RenderGraph::reset()
// invalidates every outstanding handle. Passes re-declare their resources in setup() on each
// rebuild, which is exactly when fresh handles are handed out -- never cache a handle across
// frames, only within the setup()/execute() cycle of one compiled graph.
template<typename T>
class RenderResourceHandle {
   public:
    RenderResourceHandle() = default;  // an undeclared handle: valid() == false

    bool valid() const { return m_index != kInvalidIndex; }
    explicit operator bool() const { return valid(); }
    uint32_t index() const { return m_index; }

    bool operator==(const RenderResourceHandle& o) const { return m_index == o.m_index; }
    bool operator!=(const RenderResourceHandle& o) const { return !(*this == o); }

   private:
    friend class RenderGraph;
    friend class RenderGraphBuilder;
    friend class PassContext;

    static constexpr uint32_t kInvalidIndex = std::numeric_limits<uint32_t>::max();
    explicit RenderResourceHandle(uint32_t index) : m_index(index) {}

    uint32_t m_index = kInvalidIndex;
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

    // Imported resources are backed by memory the graph does not own (see
    // RenderGraph::importResource); compile() never allocates over them.
    void markImported() { m_imported = true; }
    bool isImported() const { return m_imported; }

private:
    std::string m_name;
    ResourceDescriptor m_descriptor;
    std::shared_ptr<void> m_physical_resource;
    bool m_imported = false;
};

// Represents a single render pass in the graph.
//
// The string-named declaration API below is deprecated and kept only for the engine's internal
// geometry/present wiring; it is removed in T10. Application code (and new engine passes) should go
// through IRenderPass + RenderGraphBuilder (see RenderFeature.h), which name resources with typed
// handles instead: a name cannot be mistyped, and a resource cannot be read as the wrong type.
// These methods are not marked [[deprecated]] because the internal wiring still calls them.
class RenderGraphPass {
public:
    using ExecuteCallback = std::function<void(const RenderGraphPass&)>;

    RenderGraphPass(const std::string& name) : m_name(name) {}

    // Resource declaration API (deprecated, see the class comment)
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
    
    // Reset graph for rebuilding. Invalidates every outstanding RenderResourceHandle: the resource
    // table is rebuilt from scratch, so passes must re-declare their resources in setup().
    //
    // Transient resources this compile owned are handed to the pool first, so the next compile
    // reuses them instead of allocating: a rebuild whose descriptors are unchanged allocates
    // nothing, and the framebuffer count stays flat. Whatever the next compile doesn't claim is
    // dropped at the end of allocateResources().
    void reset() {
        for (const auto& resource : m_resource_table) {
            const auto& desc = resource->getDescriptor();
            if (resource->isImported() || !desc.is_transient || !resource->getPhysicalResource()) {
                continue;  // not ours to recycle, or must keep its contents
            }
            m_pool.push_back({desc.type, desc.width, desc.height, desc.format, resource->getPhysicalResource()});
        }
        m_passes.clear();
        m_sorted_passes.clear();
        m_resources.clear();
        m_resource_table.clear();
        m_name_to_index.clear();
        m_dependencies.clear();
        m_output_resource.clear();
    }

    // Name of the resource that must reach the screen (the backbuffer/output). Passes that don't
    // contribute to producing it are culled by compile(). If unset, every pass is kept.
    void setOutput(const std::string& resource_name) { m_output_resource = resource_name; }

    // Typed overload: declare the graph's output by handle rather than by name.
    template<typename T>
    void setOutput(RenderResourceHandle<T> handle) {
        m_output_resource = resourceName(handle.index());
    }

    // The physical resource behind the declared output; null if no output was declared or it has no
    // backing. This is what a renderer presents: the graph decides what reaches the screen, rather
    // than the renderer reaching into whichever pass it assumes produced it. Valid after compile().
    template<typename T>
    std::shared_ptr<T> output() const {
        auto it = m_name_to_index.find(m_output_resource);
        if (it == m_name_to_index.end()) {
            return nullptr;
        }
        return m_resource_table[it->second]->getPhysicalResourceAs<T>();
    }

    // --- Typed resource table ---------------------------------------------------------------
    // Register a resource and return its table index (the value behind a RenderResourceHandle).
    // Idempotent by name: re-declaring an existing name returns the existing index, which is what
    // keeps the typed layer and the (internal) string layer pointing at the same resource.
    uint32_t declareResource(const std::string& name, const ResourceDescriptor& desc) {
        auto it = m_name_to_index.find(name);
        if (it != m_name_to_index.end()) {
            return it->second;
        }
        const auto index = static_cast<uint32_t>(m_resource_table.size());
        auto resource = std::make_shared<RenderGraphResource>(name, desc);
        m_resource_table.push_back(resource);
        m_name_to_index.emplace(name, index);
        m_resources[name] = std::move(resource);  // keep the string-layer map in sync
        return index;
    }

    // Declare a resource whose physical backing is owned outside the graph (e.g. the geometry
    // pass's framebuffer) and return a typed handle to it. This is how the engine hands existing
    // targets to features; compile() will not try to allocate over an imported resource.
    template<typename T>
    RenderResourceHandle<T> importResource(const std::string& name, const std::shared_ptr<T>& physical) {
        ResourceDescriptor desc;
        desc.type = ResourceTypeOf<T>::value;
        desc.is_transient = false;  // externally owned: never aliased or pooled
        desc.debug_name = name;
        const auto index = declareResource(name, desc);
        m_resource_table[index]->setPhysicalResource(physical);
        m_resource_table[index]->markImported();
        return RenderResourceHandle<T>{index};
    }

    // The table entry behind a handle index, or nullptr if the index is out of range.
    RenderGraphResource* resourceAt(uint32_t index) const {
        return index < m_resource_table.size() ? m_resource_table[index].get() : nullptr;
    }

    // The name a handle's resource was declared under. Throws for an invalid/undeclared handle --
    // the runtime backstop for the one hole the type system leaves (a default-constructed handle).
    const std::string& resourceName(uint32_t index) const {
        if (index >= m_resource_table.size()) {
            throw std::runtime_error("RenderGraph: use of an undeclared (default-constructed) resource handle");
        }
        return m_resource_table[index]->getName();
    }

    // Get a resource by name.
    // Deprecated (removed in T10): prefer the typed handle API -- a mistyped name returns null
    // here and crashes at draw time, where an undeclared handle cannot be formed at all.
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
        // topologicalSortVisit pushes a pass only after its dependencies (the passes producing
        // what it reads), so m_sorted_passes is already in execution order -- dependencies first.
        // (The previous std::reverse here inverted that, running passes back-to-front; it was
        // never observed because the graph had no live consumer.)
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
        // Bridge the string layer into the resource table: passes that declared creates by name
        // (the internal geometry/present wiring) get a table entry, exactly as create<T>() does.
        // Idempotent, so resources already declared through the typed API are left alone.
        for (const auto& pass : m_passes) {
            for (const auto& [name, desc] : pass->getCreates()) {
                declareResource(name, desc);
            }
        }

        // Give every declared resource a physical backing: reuse a pooled one where the descriptor
        // matches, otherwise ask the factory for a new one. Imported resources arrive with theirs.
        for (const auto& resource : m_resource_table) {
            // Never allocate over a resource the graph doesn't own, even if the import was null.
            if (resource->isImported() || resource->getPhysicalResource()) {
                continue;
            }
            const auto& desc = resource->getDescriptor();
            if (auto pooled = takeFromPool(desc)) {
                resource->setPhysicalResource(std::move(pooled));
                continue;
            }
            if (!m_factory) {
                continue;  // graph-logic-only use (tests): resources stay virtual
            }
            switch (desc.type) {
                case ResourceType::RenderTarget:
                    resource->setPhysicalResource(m_factory->createFramebuffer({desc.width, desc.height, 1}));
                    break;
                case ResourceType::Texture2D:
                    resource->setPhysicalResource(m_factory->createTexture2D(desc.width, desc.height, desc.format));
                    break;
                case ResourceType::TextureCube:
                case ResourceType::Buffer:
                    // No descriptor-based creation for these yet: GraphicsFactory can only build a
                    // cube map from a loaded asset, and the engine has no backend-agnostic Buffer
                    // type for a graph resource to map onto. Such a resource stays virtual (null
                    // physical) rather than being silently mis-allocated.
                    break;
            }
        }

        // Anything the new graph didn't claim is released here rather than held forever: a resize
        // changes descriptors, so the previous sizes would never match again and would just pin
        // GPU memory.
        m_pool.clear();

        // Cache the physical resources into every pass that declared them.
        for (const auto& pass : m_passes) {
            for (const auto& [name, desc] : pass->getCreates()) {
                cacheInto(*pass, name);
            }
            for (const auto& read : pass->getReads()) {
                cacheInto(*pass, read);
            }
            for (const auto& write : pass->getWrites()) {
                cacheInto(*pass, write);
            }
        }
    }

    void cacheInto(RenderGraphPass& pass, const std::string& name) {
        auto it = m_resources.find(name);
        if (it != m_resources.end()) {
            pass.cacheResource(name, it->second->getPhysicalResource());
        }
    }

    // Claim a pooled resource matching `desc` exactly (same type/size/format are interchangeable),
    // removing it from the pool. Null when nothing matches.
    std::shared_ptr<void> takeFromPool(const ResourceDescriptor& desc) {
        auto it = std::find_if(m_pool.begin(), m_pool.end(), [&desc](const PooledResource& p) {
            return p.type == desc.type && p.width == desc.width && p.height == desc.height && p.format == desc.format;
        });
        if (it == m_pool.end()) {
            return nullptr;
        }
        auto physical = std::move(it->physical);
        m_pool.erase(it);
        return physical;
    }
    
    void cullUnusedPasses() {
        // Without a declared output there is nothing to cull against -- keep every pass.
        if (m_output_resource.empty()) {
            return;
        }

        // Mark the passes that actually contribute to the output: start from whoever writes/creates
        // the output resource, then walk their dependencies (the passes producing what they read)
        // transitively. Anything not reached is dead and is dropped from the execution order.
        std::unordered_set<RenderGraphPass*> live;
        std::vector<RenderGraphPass*> worklist;
        for (const auto& pass : m_passes) {
            const auto& writes = pass->getWrites();
            const auto& creates = pass->getCreates();
            const bool produces_output = std::find(writes.begin(), writes.end(), m_output_resource) != writes.end() ||
                                         creates.find(m_output_resource) != creates.end();
            if (produces_output && live.insert(pass.get()).second) {
                worklist.push_back(pass.get());
            }
        }
        while (!worklist.empty()) {
            auto* pass = worklist.back();
            worklist.pop_back();
            auto it = m_dependencies.find(pass);
            if (it != m_dependencies.end()) {
                for (auto* dep : it->second) {
                    if (live.insert(dep).second) {
                        worklist.push_back(dep);
                    }
                }
            }
        }

        std::vector<RenderGraphPass*> kept;
        kept.reserve(m_sorted_passes.size());
        for (auto* pass : m_sorted_passes) {
            if (live.count(pass) > 0) {
                kept.push_back(pass);
            }
        }
        m_sorted_passes = std::move(kept);
    }

private:
    std::shared_ptr<GraphicsFactory> m_factory;
    std::vector<std::unique_ptr<RenderGraphPass>> m_passes;
    std::vector<RenderGraphPass*> m_sorted_passes;
    // The resource table: handles are indices into this. Rebuilt by reset(), which is what
    // invalidates outstanding handles.
    std::vector<std::shared_ptr<RenderGraphResource>> m_resource_table;
    std::unordered_map<std::string, uint32_t> m_name_to_index;
    // The same resources keyed by name, for the internal string layer (removed in T10).
    std::unordered_map<std::string, std::shared_ptr<RenderGraphResource>> m_resources;
    std::unordered_map<RenderGraphPass*, std::vector<RenderGraphPass*>> m_dependencies;
    std::string m_output_resource;

    // Transient resources released by the last reset(), available for the next compile to reuse.
    // Survives reset() by design; drained at the end of allocateResources().
    struct PooledResource {
        ResourceType type;
        uint32_t width;
        uint32_t height;
        TextureFormat format;
        std::shared_ptr<void> physical;
    };
    std::vector<PooledResource> m_pool;
};

}  // namespace ICE
