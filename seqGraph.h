#ifndef MGNR_SEQ_GRAPH
#define MGNR_SEQ_GRAPH
#include "melody2chord.h"
#include <memory>
#include <set>
namespace mgnr::seqGraph {

struct node{
    int index = 0;
    std::vector<int> seq{};
    std::set<std::tuple<node*,float> > links{};//{{节点,距离},...}
    long iterator_flag[16];//遍历的标识
    long areaMark=-1;//用于标识不相连通的区域
    inline node(){
        for(int i=0;i<16;++i){
            iterator_flag[i] = 0;
        }
    }
};
struct graph{
    std::map<int,std::unique_ptr<node>> nodes;
    std::map<std::vector<int>,std::unique_ptr<node>> indexer;
};

inline void add(graph & self,const std::vector<int> & seq){}

}
#endif