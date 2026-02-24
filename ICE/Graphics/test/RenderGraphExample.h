//
// Example: How to use the Render Graph System
// This demonstrates a multi-pass rendering setup with shadows and post-processing
//

#include "RenderGraph.h"
#include "ForwardRenderer.h"

namespace ICE {

// Example: Setting up a render graph with multiple passes
class RenderGraphExample {
public:
    void setupRenderGraph(RenderGraph& graph, uint32_t width, uint32_t height) {
        
        // ===== Shadow Pass =====
        auto& shadow_pass = graph.addPass("ShadowPass");
        shadow_pass.create("ShadowMap", ResourceDescriptor{
            .type = ResourceType::RenderTarget,
            .width = 2048,
            .height = 2048,
            .format = GL_DEPTH_COMPONENT,
            .is_transient = false,  // Keep for main pass
            .debug_name = "Shadow Map"
        });
        shadow_pass.write("ShadowMap");
        shadow_pass.setExecuteCallback([this](const RenderGraphPass& pass) {
            auto shadow_fb = pass.getResource<Framebuffer>("ShadowMap");
            shadow_fb->bind();
            // Render scene from light's perspective
            renderShadowMap();
        });
        
        // ===== Main Geometry Pass =====
        auto& geometry_pass = graph.addPass("GeometryPass");
        geometry_pass.create("SceneColor", ResourceDescriptor{
            .type = ResourceType::RenderTarget,
            .width = width,
            .height = height,
            .format = GL_RGBA16F,
            .is_transient = true,  // Can be reused after post-processing
            .debug_name = "Scene Color"
        });
        geometry_pass.read("ShadowMap");  // Depends on shadow pass
        geometry_pass.write("SceneColor");
        geometry_pass.setExecuteCallback([this](const RenderGraphPass& pass) {
            auto scene_fb = pass.getResource<Framebuffer>("SceneColor");
            auto shadow_map = pass.getResource<Framebuffer>("ShadowMap");
            
            scene_fb->bind();
            // Bind shadow map and render geometry
            renderGeometry(shadow_map);
        });
        
        // ===== Bloom Extract Pass =====
        auto& bloom_extract = graph.addPass("BloomExtract");
        bloom_extract.create("BloomTexture", ResourceDescriptor{
            .type = ResourceType::RenderTarget,
            .width = width / 4,  // Quarter resolution
            .height = height / 4,
            .format = GL_RGBA16F,
            .is_transient = true,
            .debug_name = "Bloom"
        });
        bloom_extract.read("SceneColor");
        bloom_extract.write("BloomTexture");
        bloom_extract.setExecuteCallback([this](const RenderGraphPass& pass) {
            auto bloom_fb = pass.getResource<Framebuffer>("BloomTexture");
            auto scene = pass.getResource<Framebuffer>("SceneColor");
            
            bloom_fb->bind();
            // Extract bright areas
            extractBloom(scene);
        });
        
        // ===== Tone Mapping & Composite Pass =====
        auto& tonemap_pass = graph.addPass("ToneMappingPass");
        tonemap_pass.create("FinalColor", ResourceDescriptor{
            .type = ResourceType::RenderTarget,
            .width = width,
            .height = height,
            .format = GL_RGBA8,
            .is_transient = false,  // Final output
            .debug_name = "Final Output"
        });
        tonemap_pass.read("SceneColor");
        tonemap_pass.read("BloomTexture");
        tonemap_pass.write("FinalColor");
        tonemap_pass.setExecuteCallback([this](const RenderGraphPass& pass) {
            auto final_fb = pass.getResource<Framebuffer>("FinalColor");
            auto scene = pass.getResource<Framebuffer>("SceneColor");
            auto bloom = pass.getResource<Framebuffer>("BloomTexture");
            
            final_fb->bind();
            // Combine scene + bloom, apply tone mapping
            compositeFinal(scene, bloom);
        });
        
        // Compile the graph (resolves dependencies, allocates resources)
        graph.compile();
    }
    
    void render(RenderGraph& graph) {
        // Execute all passes in dependency order
        graph.execute();
        
        // Get final output
        auto final_resource = graph.getResource("FinalColor");
        auto final_fb = final_resource->getPhysicalResourceAs<Framebuffer>();
        
        // Blit to screen or use for further processing
        blitToScreen(final_fb);
    }

private:
    void renderShadowMap() {
        // Implementation: render scene from light perspective
    }
    
    void renderGeometry(std::shared_ptr<Framebuffer> shadow_map) {
        // Implementation: render scene with shadows
    }
    
    void extractBloom(std::shared_ptr<Framebuffer> scene) {
        // Implementation: extract bright areas
    }
    
    void compositeFinal(std::shared_ptr<Framebuffer> scene, 
                       std::shared_ptr<Framebuffer> bloom) {
        // Implementation: tone mapping + bloom composite
    }
    
    void blitToScreen(std::shared_ptr<Framebuffer> fb) {
        // Implementation: blit to default framebuffer
    }
};

}  // namespace ICE
