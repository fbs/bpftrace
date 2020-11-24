#include <ostream>
#include <pass_manager.h>

#include "bpftrace.h"
#include "printer.h"

namespace bpftrace {
namespace ast {

namespace {
void print(Node *root, std::string &name, std::ostream &out)
{
  out << "\nAST after: " << name << std::endl;
  out << "-------------------\n";
  ast::Printer printer(out);
  printer.print(root);
  out << std::endl;
}
} // namespace

void PassManager::AddPass(Pass p)
{
  passes_.push_back(std::move(p));
}

PMResult PassManager::Run(Node &n, PassContext &ctx)
{
  Node * root = &n;
  for (auto &pass : passes_)
  {
    auto result = pass.Run(*root, ctx);
    if (!result.success)
    {
      return {
        .root = nullptr,
        .success = false,
        .failed_pass = pass.name,
        .error = result.error,
      };
    }
    delete root;
    root = result.root;
    if (bt_debug != DebugLevel::kNone)
      print(root, pass.name, std::cout);
  }
  return { .root = root, .success = true };
}

} // namespace ast
} // namespace bpftrace
