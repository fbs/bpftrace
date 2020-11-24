#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <vector>
#include <optional>

namespace bpftrace{

class BPFtrace;

namespace ast {

class Node;

/**
   Result of a pass run
 */
struct PassResult
{
public:
  bool success;
  std::string error;
  Node * root = nullptr;
};

/**
   Context/config for passes
*/
struct PassContext
{
  BPFtrace &b;
};

using PassFPtr = std::function<PassResult(Node &, const PassContext &)>;

/*
  Base pass
*/
class Pass
{
public:
  Pass() = delete;
  Pass(std::string name, PassFPtr fn) : fn_(fn), name(name){};

  virtual ~Pass() = default;

  PassResult Run(Node &root, const PassContext &ctx)
  {
    return fn_(root, ctx);
  };

private:
  PassFPtr fn_;

public:
  std::string name;
};

/**
   Read only diagnostic AST passes
 */
class AnalysePass : public Pass
{
public:
  AnalysePass(std::string name, PassFPtr fn) : Pass(name, fn) {};
};

/**
   AST modifying passes
 */
class MutatePass : public Pass
{
public:
  MutatePass(std::string name, PassFPtr fn) : Pass(name, fn) {};
};

struct PMResult {
  Node * root;
  bool success;
  std::string failed_pass;
  std::string error;
};

class PassManager
{
public:
  PassManager() = default;

  void AddPass(Pass p);
  PMResult Run(Node &n, PassContext &ctx);

private:
  std::vector<Pass> passes_;
};

} // namespace ast
} // namespace bpftrace
