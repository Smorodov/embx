#include "plan/LayoutGraph.h"
#include <limits>
#include <sstream>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace embx::plan {
namespace {

void collect(const Expr* e, std::vector<const Expr*>& ids) {
    if (!e) return;
    if (e->kind == core::ExprKind::Identifier) ids.push_back(e);
    collect(e->left.get(), ids); collect(e->right.get(), ids);
}

std::string exprText(const Expr* e) {
    if (!e) return {};
    if (e->kind == core::ExprKind::Identifier || e->kind == core::ExprKind::Literal) return e->text;
    if (e->kind == core::ExprKind::Unary) return e->op + exprText(e->left.get());
    if (e->kind == core::ExprKind::Binary)
        return "(" + exprText(e->left.get()) + " " + e->op + " " + exprText(e->right.get()) + ")";
    return e->text;
}

bool addExprEdges(const Expr* e, size_t from, LayoutGraph& g, std::string& error) {
    std::vector<const Expr*> ids; collect(e, ids);
    for (const auto* id : ids) {
        if ((id->text == "$next" && id->reference.id == core::BuiltinNextSymbolId) ||
            (id->text == "$size_in_bytes" && id->reference.id == core::BuiltinSizeInBytesSymbolId) ||
            (id->text == "$min_size_in_bytes" && id->reference.id == core::BuiltinMinSizeInBytesSymbolId) ||
            (id->text == "$max_size_in_bytes" && id->reference.id == core::BuiltinMaxSizeInBytesSymbolId)) continue;
        if (!id->reference.valid()) {
            error = "layout dependency contains unresolved identifier: " + id->text;
            return false;
        }
        g.edges.push_back({from, id->reference.id, exprText(e)});
    }
    return true;
}

bool walkMembers(const std::vector<std::unique_ptr<Op>>& members, const std::string& owner,
                 LayoutGraph& g, std::string& error) {
    for (const auto& op : members) {
        if (!op) { error = owner + ": null layout operation"; return false; }
        std::string name = "<op>";
        core::SymbolId symbol = core::InvalidSymbolId;
        // Every layout operation gets a graph node as the source of its
        // dependency edges. Named field-like operations carry their canonical
        // SymbolId; structural operations remain synthetic nodes with an
        // invalid SymbolId. This keeps `from` a node id while `to` remains the
        // canonical semantic SymbolId of the dependency.
        const size_t id = g.nodes.size();
        if (op->kind == OpKind::Virtual) {
            const auto& v = static_cast<const Virtual&>(*op);
            name = v.name;
            symbol = v.symbol;
        } else if (op->kind == OpKind::Alias) {
            const auto& a = static_cast<const FieldAlias&>(*op);
            name = a.name;
            symbol = a.symbol;
        } else if (op->kind == OpKind::Field) {
            const auto& f = static_cast<const Field&>(*op);
            name = f.name;
            symbol = f.symbol;
        } else if (op->kind == OpKind::Block) {
            name = static_cast<const Block&>(*op).name;
        } else if (op->kind == OpKind::Variant) {
            name = static_cast<const Variant&>(*op).name;
        } else if (op->kind == OpKind::At) {
            name = "<at>";
        }
        g.nodes.push_back({id, symbol, name, owner, symbol != core::InvalidSymbolId});

        if (op->kind == OpKind::Virtual) {
            const auto& v = static_cast<const Virtual&>(*op);
            if (!addExprEdges(v.expression.get(), id, g, error)) return false;
        } else if (op->kind == OpKind::Alias) {
            const auto& a = static_cast<const FieldAlias&>(*op);
            if (a.target == core::InvalidSymbolId) { error = "layout dependency contains unresolved alias target: " + a.name; return false; }
            g.edges.push_back({id, a.target, a.targetName});
        } else if (op->kind == OpKind::Field) {
            const auto& f = static_cast<const Field&>(*op);
            for (const auto& d : f.type.dimensions)
                if (d.expression && !addExprEdges(d.expression.get(), id, g, error)) return false;
        } else if (op->kind == OpKind::Bits) {
            const auto& b = static_cast<const Bits&>(*op);
            for (const auto& bf : b.fields) {
                if (bf.symbol == core::InvalidSymbolId) continue;
                const size_t bid = g.nodes.size();
                g.nodes.push_back({bid, bf.symbol, bf.name, owner + ".<bits>", true});
                for (const auto& d : bf.type.dimensions)
                    if (d.expression && !addExprEdges(d.expression.get(), bid, g, error)) return false;
            }
        } else if (op->kind == OpKind::Conditional) {
            const auto& c = static_cast<const Conditional&>(*op);
            if (!addExprEdges(c.condition.get(), id, g, error)) return false;
            if (!walkMembers(c.thenMembers, owner + ".<if>", g, error)) return false;
            if (!walkMembers(c.elseMembers, owner + ".<else>", g, error)) return false;
        } else if (op->kind == OpKind::Variant) {
            const auto& v = static_cast<const Variant&>(*op);
            if (!addExprEdges(v.discriminator.get(), id, g, error)) return false;
            for (const auto& c : v.cases) {
                if (c.type) {
                    for (const auto& d : c.type->dimensions)
                        if (d.expression && !addExprEdges(d.expression.get(), id, g, error)) return false;
                } else if (!walkMembers(c.members, owner + "." + name, g, error)) return false;
            }
        } else if (op->kind == OpKind::Block) {
            const auto& b = static_cast<const Block&>(*op);
            if (!addExprEdges(b.size.get(), id, g, error)) return false;
            if (!walkMembers(b.members, owner + "." + name, g, error)) return false;
        } else if (op->kind == OpKind::At) {
            const auto& a = static_cast<const At&>(*op);
            if (!addExprEdges(a.offset.get(), id, g, error)) return false;
            if (!walkMembers(a.members, owner + ".<at>", g, error)) return false;
        } else if (op->kind == OpKind::Align) {
            const auto& a = static_cast<const Align&>(*op);
            if (!addExprEdges(a.alignment.get(), id, g, error)) return false;
        } else if (op->kind == OpKind::Callback) {
            const auto& c = static_cast<const Callback&>(*op);
            for (const auto& e : c.args)
                if (!addExprEdges(e.get(), id, g, error)) return false;
        }
    }
    return true;
}

bool safeType(const Module& p, const Type& t, std::unordered_set<std::string>& seen, std::string& error) {
    std::size_t structIndex = 0;
    bool found = false;
    if (t.reference.valid()) {
        auto it = p.structIndexBySymbol.find(t.reference.id);
        if (it != p.structIndexBySymbol.end()) { structIndex = it->second; found = true; }
    }
    if (found) {
        if (!t.reference.valid()) { error = "named type has no SymbolId: " + t.name; return false; }
        const std::string visitKey = std::to_string(t.reference.id);
        if (!seen.insert(visitKey).second) { error = "cyclic type dependency: " + t.name; return false; }
        const auto& s = p.structs[structIndex];
        std::function<bool(const std::vector<std::unique_ptr<Op>>&, const std::string&)> walk =
            [&](const auto& members, const std::string& owner) -> bool {
                for (const auto& op : members) {
                    if (!op) { error = "null operation in " + owner; return false; }
                    switch (op->kind) {
                    case OpKind::Virtual: break;
                    case OpKind::Alias: break;
                    case OpKind::Field: if (!safeType(p, static_cast<const Field&>(*op).type, seen, error)) return false; break;
                    case OpKind::Block: if (!walk(static_cast<const Block&>(*op).members, owner + "." + static_cast<const Block&>(*op).name)) return false; break;
                    case OpKind::At: if (!walk(static_cast<const At&>(*op).members, owner + ".<at>")) return false; break;
                    case OpKind::Conditional: {
                        const auto& c = static_cast<const Conditional&>(*op);
                        if (!walk(c.thenMembers, owner + ".<if>")) return false;
                        if (!walk(c.elseMembers, owner + ".<else>")) return false;
                        break;
                    }
                    case OpKind::Variant: {
                        const auto& v = static_cast<const Variant&>(*op);
                        for (const auto& c : v.cases) {
                            if (c.type && !safeType(p, *c.type, seen, error)) return false;
                            if (!walk(c.members, owner + "." + v.name + ".case")) return false;
                        }
                        if (v.defaultType && !safeType(p, *v.defaultType, seen, error)) return false;
                        if (!walk(v.defaultMembers, owner + "." + v.name + ".default")) return false;
                        break;
                    }
                    case OpKind::Bits: { const auto& b=static_cast<const Bits&>(*op); if(b.totalBits==0||b.storageBytes==0){error="invalid empty bits layout in "+owner;return false;} break; }
                    case OpKind::Align: { const auto& a=static_cast<const Align&>(*op); if(a.staticAlignment&&*a.staticAlignment==0){error="static alignment must be non-zero in "+owner;return false;} break; }
                    case OpKind::Callback: break;
                    }
                }
                return true;
            };
        const bool ok = walk(s.members, s.name);
        seen.erase(visitKey);
        return ok;
    }
    if (t.kind == core::TypeKind::Named) {
        // Enums are scalar wire values. They are terminal layout types for
        // static-safety traversal, just like primitive integers. Without this
        // case a perfectly valid field such as `kind: Kind` was reported as an
        // unresolved named type after fixedTypeSize() had already resolved it.
        if (t.reference.valid() && p.enumIndexBySymbol.find(t.reference.id) != p.enumIndexBySymbol.end())
            return true;
        if (t.reference.valid()) {
            auto as = p.aliasIndexBySymbol.find(t.reference.id);
            if (as != p.aliasIndexBySymbol.end())
                return safeType(p, p.aliases[as->second].target, seen, error);
        }
        if (t.name == "bytes" || t.name == "string") return true;
        error = "layout graph encountered unresolved named type: " + t.name;
        return false;
    }
    return true;
}

} // namespace

const LayoutNode* LayoutGraph::findNode(const std::string& owner, const std::string& name) const noexcept {
    for (const auto& n : nodes) if (n.owner == owner && n.name == name) return &n;
    return nullptr;
}

const LayoutNode* LayoutGraph::findNode(core::SymbolId symbol) const noexcept {
    if (symbol == core::InvalidSymbolId) return nullptr;
    for (const auto& n : nodes) if (n.symbol == symbol) return &n;
    return nullptr;
}

bool buildLayoutGraph(const Module& plan, LayoutGraph& graph, std::string& error) {
    graph = {};
    // Runtime parameters are terminal value sources for layout expressions.
    // They are graph nodes so parameter-dependent layout edges resolve to a
    // declared SymbolId, but they have no outgoing dependencies of their own.
    for (const auto& p : plan.parameters) {
        if (p.symbol == core::InvalidSymbolId) {
            error = "parameter has invalid SymbolId: " + p.name;
            return false;
        }
        const size_t id = graph.nodes.size();
        graph.nodes.push_back({id, p.symbol, p.name, "<module>", false});
    }
    for (const auto& c : plan.constants) {
        const size_t id = graph.nodes.size();
        graph.nodes.push_back({id, c.symbol, c.name, "<module>", false});
    }
    for (const auto& e : plan.enums) {
        for (const auto& item : e.items) {
            if (item.symbol == core::InvalidSymbolId) {
                error = "enum item has invalid SymbolId: " + e.name + "::" + item.name;
                return false;
            }
            const size_t id = graph.nodes.size();
            graph.nodes.push_back({id, item.symbol, item.name, e.name, false});
        }
    }
    for (const auto& s : plan.structs) {
        if (!walkMembers(s.members, s.name, graph, error)) return false;
    }
    return validateLayoutGraph(graph, error);
}

bool validateLayoutGraph(const LayoutGraph& graph, std::string& error) {
    std::unordered_map<core::SymbolId,size_t> symbolNodes;
    for (const auto& n : graph.nodes) {
        if (n.symbol == core::InvalidSymbolId) continue;
        if (!symbolNodes.emplace(n.symbol, n.id).second) {
            error = "layout graph contains duplicate SymbolId";
            return false;
        }
    }

    std::vector<unsigned char> state(graph.nodes.size(), 0);
    std::vector<std::vector<size_t>> adj(graph.nodes.size());
    for (const auto& e : graph.edges) {
        if (e.from >= graph.nodes.size()) { error = "layout graph contains invalid source node"; return false; }
        if (e.to == core::InvalidSymbolId) { error = "layout graph contains unresolved dependency SymbolId"; return false; }
        auto it = symbolNodes.find(e.to);
        if (it == symbolNodes.end()) { error = "layout graph dependency targets undeclared SymbolId"; return false; }
        adj[e.from].push_back(it->second);
    }
    std::function<bool(size_t)> dfs = [&](size_t n) {
        if (state[n] == 1) { error = "layout dependency cycle detected at: " + graph.nodes[n].owner + "." + graph.nodes[n].name; return false; }
        if (state[n] == 2) return true;
        state[n] = 1;
        for (size_t to : adj[n]) if (!dfs(to)) return false;
        state[n] = 2; return true;
    };
    for (size_t i=0;i<graph.nodes.size();++i) if (!dfs(i)) return false;
    return true;
}

bool validateStaticLayoutSafety(const Module& plan, std::string& error) {
    std::unordered_set<std::string> seen;
    std::function<bool(const std::vector<std::unique_ptr<Op>>&, const std::string&)> walk =
        [&](const auto& members, const std::string& owner) -> bool {
            for (const auto& op : members) {
                if (!op) { error = "null operation in " + owner; return false; }
                switch (op->kind) {
                case OpKind::Virtual: break;
                case OpKind::Alias: break;
                case OpKind::Field: {
                    const auto& f = static_cast<const Field&>(*op);
                    if (!safeType(plan, f.type, seen, error)) return false;
                    break;
                }
                case OpKind::Block: {
                    const auto& b = static_cast<const Block&>(*op);
                    if (b.staticSize && *b.staticSize == 0) {
                        error = "static block size must be non-zero in " + owner;
                        return false;
                    }
                    if (!walk(b.members, owner + "." + b.name)) return false;
                    break;
                }
                case OpKind::At: {
                    const auto& a = static_cast<const At&>(*op);
                    if (a.staticOffset && *a.staticOffset > std::numeric_limits<runtime::LayoutSize>::max()) {
                        error = "static offset exceeds layout size in " + owner;
                        return false;
                    }
                    if (!walk(a.members, owner + ".<at>")) return false;
                    break;
                }
                case OpKind::Conditional: {
                    const auto& c = static_cast<const Conditional&>(*op);
                    if (!walk(c.thenMembers, owner + ".<if>")) return false;
                    if (!walk(c.elseMembers, owner + ".<else>")) return false;
                    break;
                }
                case OpKind::Variant: {
                    const auto& v = static_cast<const Variant&>(*op);
                    for (const auto& c : v.cases) {
                        if (c.type && !safeType(plan, *c.type, seen, error)) return false;
                        if (!walk(c.members, owner + "." + v.name + ".case")) return false;
                    }
                    if (v.defaultType && !safeType(plan, *v.defaultType, seen, error)) return false;
                    if (!walk(v.defaultMembers, owner + "." + v.name + ".default")) return false;
                    break;
                }
                case OpKind::Bits: {
                    const auto& b = static_cast<const Bits&>(*op);
                    if (b.totalBits == 0 || b.totalBits > 64 || b.storageBytes == 0) {
                        error = "invalid bits layout in " + owner;
                        return false;
                    }
                    break;
                }
                case OpKind::Align: {
                    const auto& a = static_cast<const Align&>(*op);
                    if (a.staticAlignment && *a.staticAlignment == 0) {
                        error = "static alignment must be non-zero in " + owner;
                        return false;
                    }
                    break;
                }
                case OpKind::Callback:
                    break;
                }
            }
            return true;
        };

    for (const auto& s : plan.structs)
        if (!walk(s.members, s.name)) return false;
    return true;
}

} // namespace embx::plan
