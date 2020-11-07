#pragma once

#include "bpftrace.h"
#include "visitors.h"
#include "pass_manager.h"

namespace bpftrace {
namespace ast {

class PositionalParamTransformer : public Mutator
{
public:
  PositionalParamTransformer() = default;
  PositionalParamTransformer(BPFtrace *bpftrace) : bpftrace_(bpftrace){};

  Node *visit(PositionalParameter &param);
  Node *visit(Call &call);
  Node *visit(Binop &binop);

private:
  BPFtrace *bpftrace_ = nullptr;
  bool in_str_ = false;
};

Pass CreatePositionalParamPass();

} // namespace ast
} // namespace bpftrace
