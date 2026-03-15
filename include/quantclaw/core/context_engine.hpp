// Copyright 2025 QuantClaw Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "quantclaw/providers/llm_provider.hpp"

namespace quantclaw {

// Result of context assembly.
struct AssembleResult {
  std::vector<Message> messages;
  int estimated_tokens = 0;
};

// Result of context compaction.
struct CompactResult {
  bool compacted = false;
  std::string reason;
  int messages_before = 0;
  int messages_after = 0;
};

// Summary function type: given a list of JSON messages, return a summary string.
using CompactionSummaryFn =
    std::function<std::string(const std::vector<nlohmann::json>&)>;

// Abstract context engine interface.
//
// Lifecycle methods modeled after OpenClaw's ContextEngine:
//   bootstrap → assemble → [compact] → after_turn
//
// The default implementation wraps ContextPruner + SessionCompaction.
// Custom engines can implement alternative strategies (e.g., embedding-based
// retrieval, dynamic chunking, domain-specific context assembly).
class ContextEngine {
 public:
  virtual ~ContextEngine() = default;

  // Human-readable engine name (for logging).
  virtual std::string Name() const = 0;

  // Initialize context for a session (called once per session).
  virtual void Bootstrap(const std::string& session_key) {
    (void)session_key;
  }

  // Assemble the final message context from history + system prompt + user
  // message, respecting the token budget. Returns the assembled messages
  // ready for LLM submission.
  virtual AssembleResult Assemble(const std::vector<Message>& history,
                                  const std::string& system_prompt,
                                  const std::string& user_message,
                                  int context_window,
                                  int max_tokens) = 0;

  // Compact (compress) messages when context overflows.
  // Called during overflow recovery; returns compacted message list.
  virtual std::vector<Message> CompactOverflow(
      const std::vector<Message>& messages,
      const std::string& system_prompt,
      int keep_recent) = 0;

  // Post-turn bookkeeping (called after assistant response is complete).
  virtual void AfterTurn(const std::vector<Message>& new_messages,
                         const std::string& session_key) {
    (void)new_messages;
    (void)session_key;
  }

  // Subagent lifecycle hooks (optional).
  virtual void OnSubagentSpawn(const std::string& parent_key,
                               const std::string& child_key) {
    (void)parent_key;
    (void)child_key;
  }
  virtual void OnSubagentEnded(const std::string& child_key) {
    (void)child_key;
  }
};

}  // namespace quantclaw
