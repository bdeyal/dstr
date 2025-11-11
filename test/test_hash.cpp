#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <chrono>

#include <cstdio>
#include <cassert>
#include <dstr/dstring.hpp>

using namespace std;


class Timer {
public:
    using Clock = std::chrono::steady_clock;
    using Duration = std::chrono::duration<double>;

    Timer(const DString& name_)
        :
        name(name_), start(Clock::now())
    {
    }

    ~Timer()
    {
        auto end = Clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << name << " took "
                  << elapsed.count() << " µs ("
                  << Duration(end - start).count() << " seconds)\n";
    }

private:
    DString name;
    Clock::time_point start;
};

void hash_dstring()
{
    std::map<unsigned long, int> collisions;
    DString word;
    std::vector<DString> v;

    size_t word_count = 0;
    size_t collision_count = 0;

    FILE* fp = fopen("/usr/share/dict/words", "r");
    while (word.fgets(fp) != EOF) {
        v.push_back(word);
    }
    fclose(fp);

    Timer t1(__func__);

    uint64_t f = 0;
    for (const auto& w : v) {
        word_count++;
        f += w.hash();
        if (++collisions[w.hash()] > 1)
            ++collision_count;
    }

    // supress warning
    //
    ((void)f);

    printf("DSTR: Word Count = %zu, Collisions = %zu, ratio = %g\n",
           word_count,
           collision_count,
           double(collision_count) / word_count);

    assert(collision_count == 0);
}

void hash_std()
{
    std::map<uint64_t, int> collisions;
    ifstream in("/usr/share/dict/words");
    std::string word;
    std::vector<std::string> v;

    size_t word_count = 0;
    size_t collision_count = 0;

    while (in >> word) {
        v.push_back(word);
    }

    Timer t1(__func__);

    uint64_t f = 0;
    for (const auto& w : v) {
        word_count++;
        f += std::hash<std::string>{}(w);
        if (++collisions[std::hash<std::string>{}(w)] > 1)
            ++collision_count;
    }
    // supress warning
    //
    ((void)f);

    printf(" STD: Word Count = %zu, Collisions = %zu, ratio = %g\n",
           word_count,
           collision_count,
           double(collision_count) / word_count);
}



int main()
{
    hash_dstring();
    hash_std();
}
