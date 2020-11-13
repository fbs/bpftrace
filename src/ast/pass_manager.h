#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

namespace bpftrace{

class BPFtrace;

namespace ast {


/**
   Result of running a pass
 */
class PassResult
{
public:
  bool success;
  std::string error;
  Node * root = nullptr;
};

/**
   Context/config to pass to a pass
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

class PassManager
{
};

} // namespace ast
} // namespace bpftrace
