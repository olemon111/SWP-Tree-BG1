#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <future>
#include "ycsb/ycsb-c.h"
#include "combotree/combotree.h"
#include "../src/combotree_config.h"
#include "fast-fair/btree.h"
#include "nvm_alloc.h"
#include "apex/apex.h"
#include "lbtree/lbtree.h"
#include "random.h"

using combotree::ComboTree;
using FastFair::btree;
using namespace std;

namespace KV
{
  class Key_t
  {
    typedef std::array<double, 1> model_key_t;

  public:
    static constexpr size_t model_key_size() { return 1; }
    static Key_t max()
    {
      static Key_t max_key(std::numeric_limits<uint64_t>::max());
      return max_key;
    }
    static Key_t min()
    {
      static Key_t min_key(std::numeric_limits<uint64_t>::min());
      return min_key;
    }

    Key_t() : key(0) {}
    Key_t(uint64_t key) : key(key) {}
    Key_t(const Key_t &other) { key = other.key; }
    Key_t &operator=(const Key_t &other)
    {
      key = other.key;
      return *this;
    }

    model_key_t to_model_key() const
    {
      model_key_t model_key;
      model_key[0] = key;
      return model_key;
    }

    friend bool operator<(const Key_t &l, const Key_t &r) { return l.key < r.key; }
    friend bool operator>(const Key_t &l, const Key_t &r) { return l.key > r.key; }
    friend bool operator>=(const Key_t &l, const Key_t &r) { return l.key >= r.key; }
    friend bool operator<=(const Key_t &l, const Key_t &r) { return l.key <= r.key; }
    friend bool operator==(const Key_t &l, const Key_t &r) { return l.key == r.key; }
    friend bool operator!=(const Key_t &l, const Key_t &r) { return l.key != r.key; }

    uint64_t key;
  };
}

namespace dbInter
{

  static inline std::string human_readable(double size)
  {
    static const std::string suffix[] = {
        "B",
        "KB",
        "MB",
        "GB"};
    const int arr_len = 4;

    std::ostringstream out;
    out.precision(2);
    for (int divs = 0; divs < arr_len; ++divs)
    {
      if (size >= 1024.0)
      {
        size /= 1024.0;
      }
      else
      {
        out << std::fixed << size;
        return out.str() + suffix[divs];
      }
    }
    out << std::fixed << size;
    return out.str() + suffix[arr_len - 1];
  }

  class FastFairDb : public ycsbc::KvDB
  {
  public:
    FastFairDb() : tree_(nullptr) {}
    FastFairDb(btree *tree) : tree_(tree) {}
    virtual ~FastFairDb() {}
    void Init()
    {
      NVM::data_init();
      std::cout << "before init btree" << std::endl;
      tree_ = new btree();
      std::cout << "init db" << std::endl;
      NVM::pmem_size = 0;
    }

    void Info()
    {
      std::cout << "NVM WRITE : " << NVM::pmem_size << std::endl;
      NVM::show_stat();
      tree_->PrintInfo();
    }

    void Close()
    {
    }
    int Put(uint64_t key, uint64_t value)
    {
      tree_->btree_insert(key, (char *)value);
      return 1;
    }
    int Get(uint64_t key, uint64_t &value)
    {
      value = (uint64_t)tree_->btree_search(key);
      return 1;
    }
    int Update(uint64_t key, uint64_t value)
    {
      tree_->btree_delete(key);
      tree_->btree_insert(key, (char *)value);
      return 1;
    }

    int Delete(uint64_t key)
    {
      tree_->btree_delete(key);
      return 1;
    }

    int Scan(uint64_t start_key, int len, std::vector<std::pair<uint64_t, uint64_t>> &results)
    {
      tree_->btree_search_range(start_key, UINT64_MAX, results, len);
      return 1;
    }
    void PrintStatic()
    {
      NVM::show_stat();
      tree_->PrintInfo();
    }

  private:
    btree *tree_;
  };

  class LBTreeDB : public ycsbc::KvDB
  {
  public:
    LBTreeDB() : tree_(nullptr) {}
    LBTreeDB(lbtree *tree) : tree_(tree) {}
    virtual ~LBTreeDB() {}
    void Init()
    {
      NVM::data_init();
      char *nvm_addr = (char *)nvmpool_alloc(4 * KB);
      tree_ = new lbtree(nvm_addr, false);
      NVM::pmem_size = 0;
    }

    void Info()
    {
      std::cout << "NVM WRITE : " << NVM::pmem_size << std::endl;
      NVM::show_stat();
      tree_->PrintInfo();
    }

    void Close()
    {
    }
    int Put(uint64_t key, uint64_t value)
    {
      tree_->insert(key, (void *)value);
      return 1;
    }
    int Get(uint64_t key, uint64_t &value)
    {
      int pos;
      value = (uint64_t)tree_->lookup(key, &pos);
      return 1;
    }
    int Update(uint64_t key, uint64_t value)
    {
      tree_->del(key);
      tree_->insert(key, (void *)value);
      return 1;
    }

    int Delete(uint64_t key)
    {
      tree_->del(key);
      return 1;
    }

    int Scan(uint64_t start_key, int len, std::vector<std::pair<uint64_t, uint64_t>> &results)
    {
      return 1;
    }
    void PrintStatic()
    {
      NVM::show_stat();
      tree_->PrintInfo();
    }

  private:
    lbtree *tree_;
  };

  class ApexDB : public ycsbc::KvDB
  {
    typedef uint64_t KEY_TYPE;
    typedef uint64_t PAYLOAD_TYPE;
    using Alloc = my_alloc::allocator<std::pair<KEY_TYPE, PAYLOAD_TYPE>>;
    // #ifdef USE_MEM
    //     using Alloc = std::allocator<std::pair<KEY_TYPE, PAYLOAD_TYPE>>;
    // #else
    //     using Alloc = NVM::allocator<std::pair<KEY_TYPE, PAYLOAD_TYPE>>;
    // #endif
    typedef alex::Apex<KEY_TYPE, PAYLOAD_TYPE, alex::AlexCompare, Alloc> apex_t;

  public:
    ApexDB() : apex_(nullptr) {}
    ApexDB(apex_t *apex) : apex_(apex) {}
    virtual ~ApexDB()
    {
      my_alloc::BasePMPool::ClosePool();
      delete apex_;
      apex_ = NULL;
    }

    void Init()
    {
      Tree<uint64_t, uint64_t> *index = nullptr;

      bool recover = my_alloc::BasePMPool::Initialize(pool_name, pool_size);
      auto index_ptr = reinterpret_cast<Tree<uint64_t, uint64_t> **>(my_alloc::BasePMPool::GetRoot(sizeof(Tree<uint64_t, uint64_t> *)));
      if (recover)
      {
        cout << "recover\n";
        index = reinterpret_cast<Tree<uint64_t, uint64_t> *>(reinterpret_cast<char *>(*index_ptr) + 48);
        new (index) alex::Apex<uint64_t, uint64_t>(recover);
      }
      else
      {
        my_alloc::BasePMPool::ZAllocate(reinterpret_cast<void **>(index_ptr), sizeof(alex::Apex<uint64_t, uint64_t>) + 64);
        index = reinterpret_cast<Tree<uint64_t, uint64_t> *>(reinterpret_cast<char *>(*index_ptr) + 48);
        new (index) alex::Apex<uint64_t, uint64_t>();
      }
      apex_ = index;
    }

    void Bulk_load(const std::pair<uint64_t, uint64_t> data[], int size)
    {
      apex_->bulk_load(data, size);
    }

    void Info()
    {
      // apex_->PrintInfo();
    }

    int Put(uint64_t key, uint64_t value)
    {
      apex_->insert(key, value);
      return 1;
    }
    int Get(uint64_t key, uint64_t &value)
    {
      apex_->search(key, &value);
      // assert(value == key);
      return 1;
    }
    int Update(uint64_t key, uint64_t value)
    {
      apex_->update(key, value);
      return 1;
    }
    int Delete(uint64_t key)
    {
      apex_->erase(key);
      return 1;
    }
    int Scan(uint64_t start_key, int len, std::vector<std::pair<uint64_t, uint64_t>> &results)
    {
      // apex_->range_scan_by_size(start_key, len, results);
      return 1;
    }
    void PrintStatic()
    {
    }

  private:
    Tree<uint64_t, uint64_t> *apex_;
  };

} // namespace dbInter
