#pragma once
#include "plan/Plan.h"
#include <string>
#include <vector>

namespace embx::plan {

struct LayoutNode {
    size_t id = 0;
    core::SymbolId symbol = core::InvalidSymbolId;
    std::string name;
    std::string owner;
    bool executable = true;
};

struct LayoutEdge {
    // The source is the graph node that owns the layout operation. The target
    // is semantic identity, not a source-language name. The target node is
    // recovered from LayoutNode::symbol during graph validation.
    size_t from = 0;
    core::SymbolId to = core::InvalidSymbolId;
    std::string expression;
};

struct LayoutGraph {
    std::vector<LayoutNode> nodes;
    std::vector<LayoutEdge> edges;

    const LayoutNode* findNode(const std::string& owner, const std::string& name) const noexcept;
    const LayoutNode* findNode(core::SymbolId symbol) const noexcept;
};

// Builds the explicit dependency graph of executable layout operations.
// Dependency targets are canonical SymbolIds from semantic resolution.
bool buildLayoutGraph(const Module& plan, LayoutGraph& graph, std::string& error);

// Validates that the graph is acyclic and all dependency edges target
// declared semantic symbols.
bool validateLayoutGraph(const LayoutGraph& graph, std::string& error);

// Performs static overflow/safety checks that can be decided without runtime
// field values. Dynamic expressions are checked again by the runtime evaluator.
bool validateStaticLayoutSafety(const Module& plan, std::string& error);

} // namespace embx::plan
