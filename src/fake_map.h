#pragma once

#include "map.h"

namespace bpftrace {

class FakeMap : public IMap
{
public:
  FakeMap(const std::string &name, const SizedType &type, const MapKey &key);
  FakeMap(const SizedType &type);
  FakeMap(enum bpf_map_type map_type);

  static int next_mapfd_;
  int mapfd_;
};

} // namespace bpftrace
