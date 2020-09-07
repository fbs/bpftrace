#pragma once

#include "imap.h"
#include <map>
#include <memory>

namespace bpftrace {

class Map : public IMap
{
public:
  Map(const std::string &name,
      const SizedType &type,
      const MapKey &key,
      int max_entries)
      : Map(name, type, key, 0, 0, 0, max_entries){};
  Map(const std::string &name,
      const SizedType &type,
      const MapKey &key,
      int min,
      int max,
      int step,
      int max_entries);
  Map(const SizedType &type);
  Map(enum bpf_map_type map_type);
  virtual ~Map() override;

  int create_map(enum bpf_map_type map_type,
                 const char *name,
                 int key_size,
                 int value_size,
                 int max_entries,
                 int flags);
};

class MapManager
{
public:
  MapManager(){};

  MapManager(const MapManager &) = delete;
  MapManager &operator=(const MapManager &) = delete;
  MapManager(MapManager &&) = delete;
  MapManager &operator=(MapManager &&) = delete;

  void Add(std::unique_ptr<IMap> map);
  IMap &Lookup(const std::string &name);
  IMap &Lookup(ssize_t id);

  IMap &operator[](ssize_t id)
  {
    return Lookup(id);
  };
  IMap &operator[](const std::string &name)
  {
    return Lookup(name);
  };

  auto begin()
  {
    return maps_by_id_.begin();
  };
  auto end()
  {
    return maps_by_id_.end();
  };

private:
  std::vector<std::shared_ptr<IMap>> maps_by_id_;
  std::map<std::string, std::shared_ptr<IMap>> maps_by_name_;
  uint32_t id_ = 1;

  uint32_t GetID()
  {
    return id_++;
  };
};

} // namespace bpftrace
