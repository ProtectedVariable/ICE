#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "RenderGraph.h"

using namespace ICE;

// These tests exercise the graph's compile logic (dependency ordering, cycle detection, culling)
// only. A null GraphicsFactory is fine because no pass creates a RenderTarget -- the only resource
// type that touches the factory -- so no GL is invoked.

static size_t indexOf(const std::vector<std::string>& v, const std::string& s) {
    return static_cast<size_t>(std::find(v.begin(), v.end(), s) - v.begin());
}

TEST(RenderGraphTest, ExecutesDependenciesFirst) {
    RenderGraph graph(nullptr);
    std::vector<std::string> order;

    auto& a = graph.addPass("A");
    a.write("x");
    a.setExecuteCallback([&](const RenderGraphPass&) { order.push_back("A"); });

    auto& b = graph.addPass("B");
    b.read("x");
    b.write("y");
    b.setExecuteCallback([&](const RenderGraphPass&) { order.push_back("B"); });

    auto& c = graph.addPass("C");
    c.read("y");
    c.setExecuteCallback([&](const RenderGraphPass&) { order.push_back("C"); });

    graph.compile();
    graph.execute();

    ASSERT_EQ(order.size(), 3u);
    EXPECT_LT(indexOf(order, "A"), indexOf(order, "B"));  // B reads what A writes
    EXPECT_LT(indexOf(order, "B"), indexOf(order, "C"));  // C reads what B writes
}

TEST(RenderGraphTest, DetectsCycles) {
    RenderGraph graph(nullptr);
    auto& a = graph.addPass("A");
    a.read("y");
    a.write("x");
    auto& b = graph.addPass("B");
    b.read("x");
    b.write("y");
    EXPECT_THROW(graph.compile(), std::runtime_error);
}

TEST(RenderGraphTest, CullsPassesNotContributingToOutput) {
    RenderGraph graph(nullptr);
    std::vector<std::string> ran;

    auto& geometry = graph.addPass("geometry");
    geometry.write("color");
    geometry.setExecuteCallback([&](const RenderGraphPass&) { ran.push_back("geometry"); });

    auto& present = graph.addPass("present");
    present.read("color");
    present.write("backbuffer");
    present.setExecuteCallback([&](const RenderGraphPass&) { ran.push_back("present"); });

    auto& debug = graph.addPass("debug");  // produces something nobody reads and isn't the output
    debug.write("debug_overlay");
    debug.setExecuteCallback([&](const RenderGraphPass&) { ran.push_back("debug"); });

    graph.setOutput("backbuffer");
    graph.compile();
    graph.execute();

    EXPECT_NE(std::find(ran.begin(), ran.end(), "geometry"), ran.end());
    EXPECT_NE(std::find(ran.begin(), ran.end(), "present"), ran.end());
    EXPECT_EQ(std::find(ran.begin(), ran.end(), "debug"), ran.end());  // culled
}

TEST(RenderGraphTest, NoOutputKeepsAllPasses) {
    RenderGraph graph(nullptr);
    int count = 0;
    auto& a = graph.addPass("A");
    a.write("x");
    a.setExecuteCallback([&](const RenderGraphPass&) { count++; });
    auto& b = graph.addPass("B");
    b.write("unused");
    b.setExecuteCallback([&](const RenderGraphPass&) { count++; });

    graph.compile();  // no setOutput() -> nothing is culled
    graph.execute();

    EXPECT_EQ(count, 2);
}
