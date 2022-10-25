#include <iostream>
#include <fstream>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <thread>
#include <getopt.h>
#include <unistd.h>
#include <cmath>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <string.h>
#include <unordered_set>
#include "db_interface.h"
#include "timer.h"
#include "util.h"
#include "random.h"
#include "nvm_alloc.h"
#include "utils.h"

using combotree::ComboTree;
using combotree::Random;
using combotree::Timer;
using ycsbc::KvDB;
using namespace dbInter;

struct operation
{
    /* data */
    uint64_t op_type;
    uint64_t op_len; // for only scan
    uint64_t key_num;
};

// 实时获取程序占用的内存，单位：kb。
size_t physical_memory_used_by_process()
{
    FILE *file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != nullptr)
    {
        if (strncmp(line, "VmRSS:", 6) == 0)
        {
            int len = strlen(line);

            const char *p = line;
            for (; std::isdigit(*p) == false; ++p)
            {
            }

            line[len - 3] = 0;
            result = atoi(p);

            break;
        }
    }

    fclose(file);
    return result;
}

template <typename T>
std::vector<T> read_data_from_osm(
    const std::string load_file,
    T (*get_data)(const std::string &) = []
    { return static_cast<T>(0); },
    std::string output = "")
{
    std::vector<T> data;
    std::set<T> unique_keys;
    std::cout << "Use: " << __FUNCTION__ << std::endl;
    const uint64_t ns = util::timing(
        [&]
        {
            std::ifstream in(load_file);
            if (!in.is_open())
            {
                std::cerr << "unable to open " << load_file << std::endl;
                exit(EXIT_FAILURE);
            }
            uint64_t id, size = 0;
            double lat, lon;
            while (!in.eof())
            {
                /* code */
                std::string tmp;
                getline(in, tmp); // 去除第一行
                while (getline(in, tmp))
                {
                    T key = get_data(tmp);
                    unique_keys.insert(key);
                    size++;
                    if (size % 10000000 == 0)
                        std::cerr << "Load: " << size << "\r";
                }
            }
            in.close();
            std::cerr << "Finshed loads ......\n";
            data.assign(unique_keys.begin(), unique_keys.end());
            std::random_shuffle(data.begin(), data.end());
            size = data.size();
            std::cerr << "Finshed random ......\n";
            std::ofstream out(output, std::ios::binary);
            out.write(reinterpret_cast<char *>(&size), sizeof(uint64_t));
            out.write(reinterpret_cast<char *>(data.data()), data.size() * sizeof(uint64_t));
            out.close();
            std::cout << "read size: " << size << ", unique data: " << unique_keys.size() << std::endl;
        });
    const uint64_t ms = ns / 1e6;
    std::cout << "generate " << data.size() << " values in " << ms << " ms (" << static_cast<double>(data.size()) / 1000 / ms << " M values/s)" << std::endl;
    return data;
}

template <typename T>
std::vector<T> load_data_from_osm(
    const std::string dataname = "/home/lbl/dataset/generate_random_osm_cellid.dat")
{
    return util::load_data<T>(dataname);
}

std::vector<uint64_t> generate_random_ycsb(size_t op_num)
{
    std::vector<uint64_t> data;
    std::unordered_set<uint64_t> unique_keys;
    data.resize(op_num);
    std::cout << "Use: " << __FUNCTION__ << std::endl;
    const uint64_t ns = util::timing(
        [&]
        {
            Random rnd(0, UINT64_MAX);
            int cnt = 0;
            while (cnt < op_num)
            {
                int data = rnd.Next();
                ;
                if (unique_keys.insert(data).second)
                    cnt++;
                if (cnt % 10000000 == 0)
                    std::cerr << "generate: " << cnt << "\r";
            }
        });

    const std::string output = "/home/lbl/dataset/generate_random_ycsb.dat";
    data.assign(unique_keys.begin(), unique_keys.end());
    random_shuffle(data.begin(), data.end());
    std::ofstream out(output, std::ios::binary);
    uint64_t size = data.size();
    out.write(reinterpret_cast<char *>(&size), sizeof(uint64_t));
    out.write(reinterpret_cast<char *>(data.data()), data.size() * sizeof(uint64_t));
    out.close();
    const uint64_t ms = ns / 1e6;
    std::cout << "generate " << data.size() << " values in " << ms << " ms (" << static_cast<double>(data.size()) / 1000 / ms << " M values/s)" << std::endl;
    for (int i = 0; i < 20; i++)
    {
        std::cout << i << ": " << data[i] << endl;
    }
    std::cout << "------------------------------" << endl;
    return data;
}

std::vector<uint64_t> generate_uniform_random(size_t op_num)
{
    std::vector<uint64_t> data;
    std::set<uint64_t> unique_keys;
    data.resize(op_num);
    std::cout << "Use: " << __FUNCTION__ << std::endl;
    const uint64_t ns = util::timing(
        [&]
        {
            Random rnd(0, UINT64_MAX);
            int cnt = 0;
            while (cnt < op_num)
            {
                int data = rnd.Next();
                ;
                if (unique_keys.count(data))
                    continue;
                else
                {
                    unique_keys.insert(data);
                    cnt++;
                }
            }
        });

    const std::string output = "/home/lbl/dataset/generate_uniform_random.dat";
    data.assign(unique_keys.begin(), unique_keys.end());
    random_shuffle(data.begin(), data.end());
    std::ofstream out(output, std::ios::binary);
    uint64_t size = data.size();
    out.write(reinterpret_cast<char *>(&size), sizeof(uint64_t));
    out.write(reinterpret_cast<char *>(data.data()), data.size() * sizeof(uint64_t));
    out.close();
    const uint64_t ms = ns / 1e6;
    std::cout << "generate " << data.size() << " values in " << ms << " ms (" << static_cast<double>(data.size()) / 1000 / ms << " M values/s)" << std::endl;
    for (int i = 0; i < 10; i++)
    {
        std::cout << data[i] << endl;
    }
    return data;
}

void show_help(char *prog)
{
    std::cout << "Usage: " << prog << " [options]" << std::endl
              << std::endl
              << "  Option:" << std::endl
              << "    --thread[-t]             thread number" << std::endl
              << "    --load-size              LOAD_SIZE" << std::endl
              << "    --put-size               PUT_SIZE" << std::endl
              << "    --get-size               GET_SIZE" << std::endl
              //   << "    --workload               WorkLoad" << std::endl
              << "    --help[-h]               show help" << std::endl;
}

int thread_num = 1;
size_t LOAD_SIZE = 2000000;
size_t PUT_SIZE = 10000000;
size_t GET_SIZE = 10000000;
int Loads_type = 0;
float theta = 0.99;

int main(int argc, char *argv[])
{
    static struct option opts[] = {
        /* NAME               HAS_ARG            FLAG  SHORTNAME*/
        {"thread", required_argument, NULL, 't'},  // 0
        {"load-size", required_argument, NULL, 0}, // 1
        {"put-size", required_argument, NULL, 0},  // 2
        {"get-size", required_argument, NULL, 0},  // 3
        {"dbname", required_argument, NULL, 0},    // 4
        // {"workload", required_argument, NULL, 0},  // 5
        {"loadstype", required_argument, NULL, 0}, // 5
        {"theta", required_argument, NULL, 0},     // 6
        {"help", no_argument, NULL, 'h'},          // 7
        {NULL, 0, NULL, 0}};

    int c;
    int opt_idx;
    std::string dbName = "fastfair";
    std::string load_file = "";

    while ((c = getopt_long(argc, argv, "t:s:dh", opts, &opt_idx)) != -1)
    {
        switch (c)
        {
        case 0:
            switch (opt_idx)
            {
            case 0:
                thread_num = atoi(optarg);
                break;
            case 1:
                LOAD_SIZE = atoi(optarg);
                break;
            case 2:
                PUT_SIZE = atoi(optarg);
                break;
            case 3:
                GET_SIZE = atoi(optarg);
                break;
            case 4:
                dbName = optarg;
                break;
            // case 5:
            //     load_file = optarg;
            //     break;
            case 5:
                Loads_type = atoi(optarg);
                break;
            case 6:
                theta = atof(optarg);
                break;
            case 7:
                show_help(argv[0]);
                return 0;
            default:
                std::cerr << "Parse Argument Error!" << std::endl;
                abort();
            }
            break;
        case 't':
            thread_num = atoi(optarg);
            break;
        case 'h':
            show_help(argv[0]);
            return 0;
        case '?':
            break;
        default:
            std::cout << (char)c << std::endl;
            abort();
        }
    }

    std::cout << "THREAD NUMBER:         " << thread_num << std::endl;
    std::cout << "LOAD_SIZE:             " << LOAD_SIZE << std::endl;
    std::cout << "PUT_SIZE:              " << PUT_SIZE << std::endl;
    std::cout << "GET_SIZE:              " << GET_SIZE << std::endl;
    std::cout << "DB  name:              " << dbName << std::endl;
    std::cout << "Loads type:            " << Loads_type << std::endl;
    // std::cout << "Workload:              " << load_file << std::endl;
    std::cout << "Theta:                 " << theta << std::endl;

    std::vector<uint64_t> data_base;
    switch (Loads_type)
    {
    case 0:
        data_base = generate_uniform_random(LOAD_SIZE);
        break;
    case 1:
        data_base = generate_random_ycsb(LOAD_SIZE);
        break;
    case 2:
        data_base = load_data_from_osm<uint64_t>("/home/lbl/dataset/generate_uniform_random.dat");
        break;
    case 3:
        data_base = load_data_from_osm<uint64_t>("/home/lbl/dataset/generate_random_ycsb.dat");
        break;
    default:
        data_base = generate_uniform_random(LOAD_SIZE);
        break;
    }

    size_t init_dram_space_use = physical_memory_used_by_process();
    std::cout << "before newdb, dram space use: " << init_dram_space_use / 1024.0 / 1024.0 << " GB" << std::endl;

    KvDB *db = nullptr;
    if (dbName == "fastfair")
    {
        db = new FastFairDb();
    }
    else if (dbName == "apex")
    {
        db = new ApexDB();
    }
    else if (dbName == "lbtree")
    {
        db = new LBTreeDB();
    }
    else
    {
        assert(false);
    }
    // cout << "getchar" << endl;
    // getchar();

    db->Init();
    Timer timer;
    uint64_t us_times;
    uint64_t load_pos = 0;
    int load_size = 0;
    {
        // Load
        std::cout << "Start loading ...." << std::endl;
        util::FastRandom ranny(18);
        timer.Record("start");
        int ops = 0;
        for (load_pos = load_size; load_pos < LOAD_SIZE; load_pos++)
        {
            db->Put(data_base[load_pos], (uint64_t)(data_base[load_pos] + 1));
            ops++;
            if (ops % 10000000 == 0)
            {
                ops = 0;
                timer.Record("stop");
                us_times = timer.Microsecond("stop", "start");
                std::cout << "Operate: " << load_pos + 1 << std::endl;
                std::cout << "dram space use: " << (physical_memory_used_by_process() - init_dram_space_use) / 1024.0 / 1024.0 << " GB" << std::endl;
                // std::cout << "dram space use: " << combotree::dram_space / 1024.0 / 1024.0 / 1024.0 << " GB" << std::endl;
                // 用于统计load过程中的nvm size和nvm write
                db->Info();
                std::cout << "write ampli: " << NVM::pmem_size * 1.0 / load_pos / 16.0 / 4.0 << std::endl;
                // 扩展性读写测试
                std::cout << "[Metic-write]: " << LOAD_SIZE << ": "
                          << "cost " << us_times / 1000000.0 << "s, "
                          << "iops " << (double)(10000000) / (double)us_times * 1000000.0 << " ." << std::endl;

                // generate random array
                vector<uint32_t> rand_pos;
                int read_size = 2000000;
                for (uint64_t i = 0; i < read_size; i++)
                {
                    rand_pos.push_back(ranny.RandUint32(0, load_pos - 1));
                }
                timer.Clear();
                timer.Record("start");
                uint64_t value = 0;
                int wrong_get = 0;
                for (uint64_t i = 0; i < read_size; i++)
                {
                    // uint64_t op_seq = ranny.RandUint32(0, load_pos - 1);
                    db->Get(data_base[rand_pos[i]], value);
                    if (value != data_base[rand_pos[i]] + 1)
                    {
                        wrong_get++;
                    }
                }
                timer.Record("stop");
                us_times = timer.Microsecond("stop", "start");
                std::cout << "wrong get: " << wrong_get << endl;
                std::cout << "[Metic-read]: " << LOAD_SIZE << ": "
                          << "cost " << us_times / 1000000.0 << "s, "
                          << "iops " << (double)(read_size) / (double)us_times * 1000000.0 << " ." << std::endl;

                timer.Clear();
                timer.Record("start");
            }
        }
        std::cerr << std::endl;

        timer.Record("stop");
        us_times = timer.Microsecond("stop", "start");
        std::cout << "[Metic-Load]: Load " << LOAD_SIZE << ": "
                  << "cost " << us_times / 1000000.0 << "s, "
                  << "iops " << (double)(LOAD_SIZE) / (double)us_times * 1000000.0 << " ." << std::endl;
    }
    load_pos = LOAD_SIZE;
    db->Info();
    // Uniform Test
    std::cout << "Start Testing Uniform Workload" << std::endl;
    // // generate random array
    // util::FastRandom ranny(18);
    vector<uint32_t> rand_pos;
    std::mt19937_64 gen(std::random_device{}());
    std::uniform_int_distribution<uint32_t> dis(0, load_pos - 1);
    for (uint64_t i = 0; i < GET_SIZE; i++)
    {
        rand_pos.push_back(ranny.RandUint32(0, load_pos - 1));
    }

    timer.Clear();
    timer.Record("start");

    int wrong_get = 0;
    uint64_t value = 0;
    for (uint64_t i = 0; i < GET_SIZE; i++)
    {
        db->Get(data_base[rand_pos[i]], value);
        if (value != data_base[rand_pos[i]] + 1)
        {
            wrong_get++;
        }
    }

    timer.Record("stop");
    std::cout << "wrong get: " << wrong_get << std::endl;
    us_times = timer.Microsecond("stop", "start");
    std::cout << "[Metic-Operate]: Operate " << GET_SIZE << " theta " << 0 << ": "
              << "cost " << us_times / 1000000.0 << "s, "
              << "iops " << (double)(GET_SIZE) / (double)us_times * 1000000.0 << " ." << std::endl;
    std::cout << "dram space use: " << (physical_memory_used_by_process() - init_dram_space_use) / 1024.0 / 1024.0 << " GB" << std::endl;

    // sleep(10);
    // // sleep(100);
    // // Zipfian Test
    // std::cout << "Start Testing Zipfian Workload" << std::endl;

    // std::vector<float> thetas = {0.6, 0.7, 0.8, 0.9, 0.95, 0.99};
    // float zipf_theta = 0.6;
    // for (int k = 0; k < thetas.size(); k++)
    // {
    //     std::default_random_engine generator_;
    //     zipfian_int_distribution<int> dis(0, load_pos - 1, thetas[i]);
    //     rand_pos.clear();
    //     for (uint64_t i = 0; i < GET_SIZE; i++)
    //     {
    //         uint32_t pos = dis(gen);
    //         rand_pos.push_back(pos);
    //     }
    //     std::random_shuffle(rand_pos.begin(), rand_pos.end());

    //     timer.Clear();
    //     timer.Record("start");

    //     wrong_get = 0;
    //     value = 0;
    //     for (uint64_t i = 0; i < GET_SIZE; i++)
    //     {
    //         db->Get(data_base[rand_pos[i]], value);
    //         if (value != data_base[rand_pos[i]] + 1)
    //         {
    //             wrong_get++;
    //         }
    //     }

    //     timer.Record("stop");
    //     std::cout << "wrong get: " << wrong_get << std::endl;
    //     us_times = timer.Microsecond("stop", "start");
    //     std::cout << "[Metic-Operate]: Operate " << GET_SIZE << " theta " << thetas[k] << ": "
    //               << "cost " << us_times / 1000000.0 << "s, "
    //               << "iops " << (double)(GET_SIZE) / (double)us_times * 1000000.0 << " ." << std::endl;
    //     std::cout << "dram space use: " << (physical_memory_used_by_process() - init_dram_space_use) / 1024.0 / 1024.0 << " GB" << std::endl;
    // }

    delete db;

    return 0;
}
