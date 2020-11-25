#include "log.h"
#include "parser.tab.hh"
#include "transform_positional_params.h"

namespace bpftrace {
namespace ast {

namespace {

Call *new_call(Call &call, ExpressionList args)
{
  auto c = call.leafcopy();
  c->vargs = new ExpressionList;
  for (auto e : args)
    c->vargs->push_back(e);
  return c;
}

} // namespace

Node *PositionalParamTransformer::visit(PositionalParameter &param)
{
  switch (param.ptype)
  {
    case PositionalParameterType::count:
      return new Integer(bpftrace_->num_params(), param.loc);
    case PositionalParameterType::positional:

      auto pstr = bpftrace_->get_param(param.n, in_str_);
      if (in_str_)
      {
        return new String(pstr, param.loc);
      }

      auto n = std::stoll(pstr, nullptr, 0);
      return new Integer(n, param.loc);
  }

  return param.leafcopy();
}

Node *PositionalParamTransformer::visit(Binop &binop)
{
  /**
     Simplify addition on string literals

     We allow str($1 +3) as a construct to offset into a string. As str() only
     sees a binop() we simplify it here
   */
  if (!in_str_ || binop.op != bpftrace::Parser::token::PLUS)
    return Mutator::visit(binop);

  auto left = Value<Expression>(binop.left);
  auto right = Value<Expression>(binop.right);

  auto str_node = dynamic_cast<String *>(left);
  auto offset_node = dynamic_cast<Integer *>(right);
  if (!str_node)
  {
    str_node = dynamic_cast<String *>(right);
    offset_node = dynamic_cast<Integer *>(left);
  }

  if (!(str_node && offset_node))
  {
    auto b = binop.leafcopy();
    b->left = left;
    b->right = right;
    return b;
  }

    auto len = str_node->str.length();
    auto offset = offset_node->n;

    if (offset < 0 || (size_t)offset > len)
    {
      LOG(ERROR, binop.loc)
          << "only addition of a single constant less or equal to the "
          << "length of $" << offset << " (which is " << len << ")"
          << " is allowed inside str()";
    }

    auto node = new String(str_node->str.substr(offset, std::string::npos), binop.loc);
    delete str_node;
    delete offset_node;
    return node;
}

Node *PositionalParamTransformer::visit(Call &call)
{
  /**
     Simplify str() with positional param calls.

     We allow some "weird" construct inside str(), e.g.
     str($1), str($1 + 3), str($1 + 3, 3) all valid.

     This simplifies these calls and replaces them with their
     string constant which makes the semantic analysis easier
     and the codegen more efficient.

     $1 == "123456789"; str($1+3, 3)  => "456"

   */
  if (call.func != "str" || !call.vargs)
    return Mutator::visit(call);

  if (call.vargs->size() > 2)
    abort(); // TODO FIxme

  /**
     with str($1, $2) we need an integer value for $2, so only
     set `in_str_` for the first expr
  */
  in_str_ = true;
  ast::Expression * arg0 = Value<Expression>(call.vargs->at(0));
  in_str_ = false;

  auto str_val = dynamic_cast<String*>(arg0);

  if (call.vargs->size() == 1)
  {
    // str("abc") => "abc"
    if (str_val)
      return  arg0;
    // str(arg0) => str(arg0)
    return new_call(call, {arg0});
  }

  auto arg1 = Value<Expression>(call.vargs->at(1));
  auto size_val = dynamic_cast<Integer*>(arg1);
  // str("abc", 3)
  if (str_val && size_val)
  {
    auto ret = new String(str_val->str.substr(0, size_val->n), call.loc);
    delete arg0;
    delete arg1;
    return ret;
  }
  // str("abc", arg0) | str(arg0, arg1)
  return new_call(call, {arg0, arg1});
}

Pass CreatePositionalParamPass(){
  auto fn = [](Node &n, PassContext &ctx) {
    auto pass = PositionalParamTransformer(&ctx.b);
    auto newroot = pass.Visit(n);
    return PassResult::Success(newroot);
  };

  return MutatePass("RemovePositionalParams", fn);
};

} // namespace ast
} // namespace bpftrace
